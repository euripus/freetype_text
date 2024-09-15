#ifndef STRINGLAYOUT_H
#define STRINGLAYOUT_H

#include <memory>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "basic_types.h"
#include "widget.h"

enum class Direction
{
    LeftToRight,
    RightToLeft,
    Down,
    Up
};

struct WidgetInfo
{
    Rect2D   geom;
    Widget * widget = nullptr;
};

using WDict = std::map<void *, WidgetInfo>;   // ptr value, widget info data

class Chain
{
public:
    Chain(Direction d)
        : dir(d),
          sstretch(0)
    {}
    virtual ~Chain() {}

    bool add(Chain * s, float stretch = 0)
    {
        if(addChain(s))
        {
            s->sstretch = stretch;
            return true;
        }
        else
            return false;
    }

    Direction direction() const { return dir; }
    float     stretch() const { return sstretch; }
    void      setStretch(float s) { sstretch = s; }

    virtual float maxSize() const = 0;
    virtual float minSize() const = 0;
    virtual void  recalc() {}

    virtual void distribute(WDict &, float pos, float space) = 0;

    virtual bool removeWidget(Widget const *) { return false; }

protected:
    virtual bool addChain(Chain * s) = 0;

private:
    Direction dir;

    float sstretch;
};

using ChainPtr = std::unique_ptr<Chain>;

struct ChainOwner
{
    std::vector<ChainPtr> chains;

    template<class T, class... Args>
    Chain * getChain(Args &&... args)
    {
        chains.push_back(std::move(std::make_unique<T>(std::forward<Args>(args)...)));
        return chains.back().get();
    }
};

class StringLayout
{
public:
    StringLayout(ChainOwner & chains_pool, Direction d, float def_border = 0);

    ~StringLayout() {}

    float border() const { return m_border; }

    void      addSpacing(float size);
    void      addStretch(float stretch = 0);
    void      addWidget(Widget * w, float stretch = 0, Align alignment = Align::center);
    void      addString(StringLayout * layout, float stretch = 0);
    Direction direction() const { return m_dir; }

    void addStrut(float size);   // Limits the perpendicular dimension of the box

    glm::vec2 resizeAll(float win_width, float win_height);

private:
    Chain * mainVerticalChain();
    Chain * mainHorizontalChain();

    Direction    m_dir;
    ChainOwner & m_chains_pool;
    Chain *      m_par_chain = nullptr;
    Chain *      m_ser_chain = nullptr;
    float        m_border;
};

class MemPool
{
public:
    using StringLayoutPtr = std::unique_ptr<StringLayout>;
    StringLayout * createNewLayout(Direction d, float def_border);

private:
    ChainOwner                   m_chains_pool;
    std::vector<StringLayoutPtr> m_box_pool;
};

#endif
