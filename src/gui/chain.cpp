#include <limits>
#include <algorithm>
#include <numeric>
#include "chain.h"

#include <iostream>

constexpr float unlimited = std::numeric_limits<float>::max();

static constexpr bool Horz(Direction dir)
{
    return dir == Direction::RightToLeft || dir == Direction::LeftToRight;
}

static constexpr Direction Perp(Direction dir)
{
    if(Horz(dir))
        return Direction::Up;
    else
        return Direction::LeftToRight;
}

static void SetWinfo(Widget * w, WDict & dict, Direction d, float p, float s)
{
    WidgetInfo & wi = dict[w];

    wi.widget = w;
    if(Horz(d))
    {
        wi.geom.m_pos.x  = p;
        wi.geom.m_size.x = s;   // setWidth
    }
    else
    {
        wi.geom.m_pos.y  = p;
        wi.geom.m_size.y = s;   // setHeight
    }
}

class SpaceChain : public Chain
{
public:
    SpaceChain(Direction d, float min, float max)
        : Chain(d),
          minsize(min),
          maxsize(max)
    {}
    // needs direction for consistency check.....
    bool addChain(Chain *) override { return false; }

    void distribute(WDict &, float, float) override {}

    float maxSize() const override { return maxsize; }
    float minSize() const override { return minsize; }

private:
    float minsize;
    float maxsize;
};

class WidChain : public Chain
{
public:
    WidChain(Direction d, Widget * w)
        : Chain(d),
          widget(w)
    {}
    bool addChain(Chain *) override { return false; }

    float minSize() const override;
    float maxSize() const override;

    bool removeWidget(Widget const * w) override
    {
        if(w == widget)
        {
            widget = nullptr;
            return true;
        }
        else
        {
            return false;
        }
    }

    void distribute(WDict & wd, float pos, float space) override
    {
        if(widget)
            SetWinfo(widget, wd, direction(), pos, space);
    }

private:
    Widget * widget;
};

class ParChain : public Chain
{
public:
    ParChain(Direction d)
        : Chain(d)
    {}

    bool addChain(Chain * s) override;

    void recalc() override;

    void distribute(WDict & wd, float pos, float space) override;
    bool removeWidget(Widget const * w) override;

    float maxSize() const override { return maxsize; }
    float minSize() const override { return minsize; }

private:
    float maxsize;
    float minsize;

    std::vector<Chain *> chains;

    float minMax() const;
    float maxMin() const;
};

class SerChain : public Chain
{
public:
    SerChain(Direction d)
        : Chain(d)
    {}

    bool addChain(Chain * s) override;

    void  recalc() override;
    void  distribute(WDict &, float, float) override;
    bool  removeWidget(Widget const * w) override;
    float maxSize() const override { return maxsize; }
    float minSize() const override { return minsize; }

private:
    float sumMax() const;
    float sumMin() const;
    float sumStretch() const;

    float maxsize;
    float minsize;

    std::vector<Chain *> chains;
};

StringLayout::StringLayout(ChainOwner & chains_pool, Direction d, float def_border)
    : m_dir(d),
      m_chains_pool(chains_pool),
      m_border(def_border)
{
    m_ser_chain = m_chains_pool.getChain<SerChain>(m_dir);
    m_par_chain = m_chains_pool.getChain<ParChain>(Perp(m_dir));
}

Chain * StringLayout::mainVerticalChain()
{
    if(Horz(m_dir))
        return m_par_chain;
    else
        return m_ser_chain;
}

Chain * StringLayout::mainHorizontalChain()
{
    if(Horz(m_dir))
        return m_ser_chain;
    else
        return m_par_chain;
}

void StringLayout::addStrut(float size)
{
    m_par_chain->add(m_chains_pool.getChain<SpaceChain>(m_par_chain->direction(), size, unlimited), 0);
}

void StringLayout::addSpacing(float size)
{
    m_ser_chain->add(m_chains_pool.getChain<SpaceChain>(m_ser_chain->direction(), size, size), 0);
}

void StringLayout::addStretch(float stretch)
{
    m_ser_chain->add(m_chains_pool.getChain<SpaceChain>(m_ser_chain->direction(), 0, unlimited), stretch);
}

void StringLayout::addWidget(Widget * widget, float stretch, Align alignment)
{
    if(widget == nullptr)
    {
        return;
    }

    if(defaultBorder())
        m_ser_chain->add(
            m_chains_pool.getChain<SpaceChain>(m_ser_chain->direction(), defaultBorder(), defaultBorder()),
            0);

    Chain * sc = m_chains_pool.getChain<SerChain>(Perp(m_dir));
    if(alignment == Align::right || alignment == Align::top || alignment == Align::center)
    {
        sc->add(m_chains_pool.getChain<SpaceChain>(sc->direction(), 0, unlimited), 0);
    }
    sc->add(m_chains_pool.getChain<WidChain>(sc->direction(), widget), 1);
    if(alignment == Align::center || alignment == Align::left || alignment == Align::bottom)
    {
        sc->add(m_chains_pool.getChain<SpaceChain>(sc->direction(), 0, unlimited), 0);
    }
    m_par_chain->add(sc, 0);

    m_ser_chain->add(m_chains_pool.getChain<WidChain>(m_ser_chain->direction(), widget), stretch);
}

void StringLayout::addString(StringLayout * layout, float stretch)
{
    if(layout == nullptr)
        return;

    if(Horz(m_dir))
    {
        m_par_chain->add(layout->mainVerticalChain());
        m_ser_chain->add(layout->mainHorizontalChain(), stretch);
    }
    else
    {
        m_par_chain->add(layout->mainHorizontalChain());
        m_ser_chain->add(layout->mainVerticalChain(), stretch);
    }
}

void StringLayout::resizeAll(float new_width, float new_height)
{
    WDict lookup_table;

    m_par_chain->recalc();
    m_ser_chain->recalc();

    float const border = defaultBorder();

    float const min_y = mainVerticalChain()->minSize() + 2 * border;
    float const min_x = mainHorizontalChain()->minSize() + 2 * border;
    float const max_y = mainVerticalChain()->maxSize();
    float const max_x = mainHorizontalChain()->maxSize();

    float const width  = std::max(min_x, std::min(new_width, max_x));
    float const height = std::max(min_y, std::min(new_height, max_y));

    mainHorizontalChain()->distribute(lookup_table, border, width - 2 * border);
    mainVerticalChain()->distribute(lookup_table, border, height - 2 * border);

    for(auto & [key, val]: lookup_table)
    {
        if(val.widget)
        {
            val.widget->setRect(val.geom);
        }
    }
}

float WidChain::minSize() const
{
    if(!widget)
        return 0;

    glm::vec2 s = widget->minimumSize();
    if(Horz(direction()))
        return s.x;   // width
    else
        return s.y;   // height
}

float WidChain::maxSize() const
{
    if(!widget)
        return unlimited;

    glm::vec2 s = widget->maximumSize();
    if(Horz(direction()))
        return s.x;   // width
    else
        return s.y;   // height
}

bool ParChain::addChain(Chain * s)
{
    if(Horz(s->direction()) != Horz(direction()))
    {
        if(Horz(direction()))
            std::cerr << "Cannot add vertical chain to horizontal serial chain";
        else
            std::cerr << "Cannot add horizontal chain to vertical serial chain";
        return false;
    }

    chains.push_back(s);
    return true;
}

void ParChain::recalc()
{
    std::for_each(std::begin(chains), std::end(chains), [](Chain * p) { p->recalc(); });

    maxsize = minMax();
    minsize = maxMin();
}

void ParChain::distribute(WDict & wd, float pos, float space)
{
    std::for_each(std::begin(chains), std::end(chains), [&](Chain * p) { p->distribute(wd, pos, space); });
}

bool ParChain::removeWidget(Widget const * w)
{
    for(auto & ch: chains)
    {
        if(ch->removeWidget(w))
        {
            return true;   // only one in a chain
        }
    }

    return false;
}

float ParChain::minMax() const
{
    float min = std::numeric_limits<float>::max();
    std::for_each(std::begin(chains), std::end(chains), [&min](Chain const * p) {
        float m = p->maxSize();
        if(m < min)
            min = m;
    });

    return min;
}

float ParChain::maxMin() const
{
    float max = 0;
    std::for_each(std::begin(chains), std::end(chains), [&max](Chain const * p) {
        float m = p->minSize();
        if(m > max)
            max = m;
    });

    return max;
}

bool SerChain::addChain(Chain * s)
{
    if(Horz(s->direction()) != Horz(direction()))
    {
        if(Horz(direction()))
            std::cerr << "Cannot add vertical chain to horizontal serial chain";
        else
            std::cerr << "Cannot add horizontal chain to vertical serial chain";
        return false;
    }

    chains.push_back(s);
    return true;
}

void SerChain::recalc()
{
    std::for_each(std::begin(chains), std::end(chains), [](Chain * p) { p->recalc(); });

    minsize = sumMin();
    maxsize = sumMax();
}

void SerChain::distribute(WDict & wd, float pos, float space)
{
    if(chains.size() == 0)
        return;

    float available = space - minSize();
    if(available < 0)
    {
        std::string msg;
        msg += "Not enough space for " + std::to_string(chains.size()) + "-item in "
               + (Horz(direction()) ? "horizontal" : "vertical") + " chain";

        std::cerr << msg << std::endl;
        available = 0;
    }

    float sf = sumStretch();

    std::vector<float> sizes(chains.size());
    std::fill(std::begin(sizes), std::end(sizes), 0.f);

    bool do_again   = true;
    int  num_chains = chains.size();
    while(do_again && num_chains)
    {
        do_again = false;
        for(int i = 0; i < static_cast<int>(chains.size()); i++)
        {
            float max_s = chains.at(i)->maxSize();
            if(sizes[i] == max_s)
                continue;

            float min_s = chains.at(i)->minSize();
            float siz   = min_s;
            if(sf > 0.0f)
                siz += available * chains.at(i)->stretch() / sf;
            else
                siz += available / num_chains;

            if(siz >= max_s)
            {
                sizes[i]   = max_s;
                available -= max_s - min_s;
                sf        -= chains.at(i)->stretch();
                num_chains--;
                do_again = true;
                break;
            }

            sizes[i] = siz;
        }
    }

    int                n = chains.size();
    std::vector<float> places(n + 1);
    places[n]  = pos + space;
    float fpos = pos;
    for(int i = 0; i < static_cast<int>(chains.size()); i++)
    {
        places[i]  = fpos;   // only give what we've got
        fpos      += sizes[i];
    }

    bool backwards = (direction() == Direction::RightToLeft || direction() == Direction::Down);

    for(int i = 0; i < static_cast<int>(chains.size()); i++)
    {
        float p = places[i];
        float s = places[i + 1] - places[i];
        if(backwards)
            p = 2 * pos + space - p - s;
        chains.at(i)->distribute(wd, p, s);
    }
}

bool SerChain::removeWidget(Widget const * w)
{
    for(int i = 0; i < static_cast<int>(chains.size()); i++)
    {
        if(chains.at(i)->removeWidget(w))
            return true;
    }
    return false;
}

float SerChain::sumMin() const
{
    auto fnc_sum = [](float a, Chain const * b) {
        return a + b->minSize();
    };

    return std::accumulate(std::begin(chains), std::end(chains), 0.f, fnc_sum);
}

float SerChain::sumMax() const
{
    auto fnc_sum = [](float a, Chain const * b) {
        return a + b->maxSize();
    };

    return std::accumulate(std::begin(chains), std::end(chains), 0.f, fnc_sum);
}

float SerChain::sumStretch() const
{
    auto fnc_sum = [](float a, Chain const * b) {
        return a + b->stretch();
    };

    return std::accumulate(std::begin(chains), std::end(chains), 0.f, fnc_sum);
}

StringLayout * MemPool::createNewLayout(Direction d, float def_border)
{
    auto new_box = std::make_unique<StringLayout>(m_chains_pool, d, def_border);
    m_box_pool.push_back(std::move(new_box));
    return m_box_pool.back().get();
}
