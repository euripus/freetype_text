struct unit
{
    int         initiative;   // in percent
    float       internal;     // initiative/100.f
    float       head_start;   // in seconds [0 ... 1/internal]
    std::string name;
};

std::array<float, 10> getTurnTimes(unit const & u, float const actual_time = 0.f)
{
    std::array<float, 10> result;
    float const           sec_for_turn = 1.f / u.internal;
    float const           curr_turn    = std::floor(actual_time / sec_for_turn);

    for(uint32_t i = curr_turn; i < curr_turn + result.size(); ++i)
    {
        result[i] = std::max(i / u.internal - head_start, 0.f);
    }

    return result;
}

//========================================================================//
enum class collection_type
{
	collumn,
	row
};

using collection = std::pair<std::vector<Widget *>, collection_type>;

std::variant<Widget *, collection> element;

using WidgetList = std::vector<element>;

auto ptr = std::get_if<Widget>(&elem);

WidgetList list = getWidgetListFromTree(...);

if(!list.empty())
{
	for(auto & el : list)
	{
		if(auto ptr = std::get_if<collection>(el); ptr != nullptr) //?????
		{
		}
		else
		{
			// matrix size 1x1
			Widget * ptr = std::get_if<Widget>(el);
		}
	}
}





