#include "uiimagemanager.h"
#include <boost/json.hpp>

void RegionDataOfUITexture::addBlock(VertexBuffer & vb, glm::vec2 & pos, glm::vec2 new_size) const
{
	if(new_size.x < static_cast<float>(left + right) || new_size.y < static_cast<float>(bottom + top))
    {
        // Error: the new rectangle size is too small
        return;
    }

    //     L              R
    //  ---------------------- 3
    //  |  |              |  |
    //  |  |              |2 |
    //  |--------------------| T
    //  |  |              |  |
    //  |  |              |  |
    //  |  |1             |  |
    //  |--------------------| B
    //  |  |              |  |
    //  |  |              |  |
    // 0----------------------
    float inv_new_width    = 1.0f / new_size.x;
    float inv_new_height   = 1.0f / new_size.y;
    float tex_coord_width  = origin.tx1.s - origin.tx0.s;
    float tex_coord_height = origin.tx1.t - origin.tx0.t;

    float x0 = pos.x;
    float y0 = pos.y;
    float s0 = origin.tx0.s;
    float t0 = origin.tx0.t;

    float x1 = x0 + static_cast<float>(left);
    float y1 = y0 + static_cast<float>(bottom);
    float s1 = s0 + inv_new_width * x1 * tex_coord_width;
    float t1 = t0 + inv_new_height * y1 * tex_coord_height;

    float x2 = new_size.x - static_cast<float>(right);
    float y2 = new_size.y - static_cast<float>(top);
    float s2 = s0 + inv_new_width * x2 * tex_coord_width;
    float t2 = t0 + inv_new_height * y2 * tex_coord_height;

    float x3 = x0 + new_size.x;
    float y3 = y0 + new_size.y;
    float s3 = origin.tx1.s;
    float t3 = origin.tx1.t;

    // add 9 rectangles to the vertex buffer
    // bottom row
    add2DRectangle(vb, x0, y0, x1, y1, s0, t0, s1, t1);
    add2DRectangle(vb, x1, y0, x2, y1, s1, t0, s2, t1);
    add2DRectangle(vb, x2, y0, x3, y1, s2, t0, s3, t1);

    // middle row
    add2DRectangle(vb, x0, y1, x1, y2, s0, t1, s1, t2);
    add2DRectangle(vb, x1, y1, x2, y2, s1, t1, s2, t2);
    add2DRectangle(vb, x2, y1, x3, y2, s2, t1, s3, t2);

    // top row
    add2DRectangle(vb, x0, y2, x1, y3, s0, t2, s1, t3);
    add2DRectangle(vb, x1, y2, x2, y3, s1, t2, s2, t3);
    add2DRectangle(vb, x2, y2, x3, y3, s2, t2, s3, t3);

    // move pen position
    pos.x += new_size.x;
}

UIImageGroup & UIImageManager::addImageGroup(std::string const & group_name)
{
    boost::json::error_code ec;
    boost::json::value jv;

    {
        size_t file_length = 0;

        std::ifstream  ifile(group_name, std::ios::in);
        std::string    file_data; 

        if(ifile.is_open())
        {
            ifile.seekg(0, std::ios_base::end);
            auto length = ifile.tellg();
            ifile.seekg(0, std::ios_base::beg);

            file_data.resize(static_cast<size_t>(length));

            ifile.read(reinterpret_cast<char *>(file_data.data()), length);

            auto success = !ifile.fail() && length == ifile.gcount();
            ifile.close();

            if(!success)
                throw std::runtime_error("File not found!");
        }
        else
            throw std::runtime_error("File not found!");

        jv = boost::json::parse(file_data, ec);
    }

    if(ec)
        throw std::runtime_error("File parsing error ");

    auto& obj = jv.get_object();
    if(!obj.empty())
    {
        std::string gr_name;
        auto new_group = std::make_unique<UIImageGroup>(*this);

        for(auto & kvp : obj)
        {
            if(kvp.key() == "gui_set")
                    gr_name = kvp.value().as_string();
            else if (kvp.key() == "images")
            {       
                // parse images
                parseImages(kvp.value(), *new_group);
            }
        }

        m_groups[gr_name] = std::move(new_group);
    }
}

void UIImageManager::parseImages(boost::json::value const & jv, UIImageGroup & group)
{
	auto& obj = jv.get_object();
    if(!obj.empty())
    {
		for(auto & kvp : obj)
        {
			std::string texture_name = kvp.key();
			std::string path;
			std::vector<int32_t> margins;
			
			for(auto & kvp2 : kvp.value().as_object())
			{
				if(kvp2.key() == "texture")
                    path = kvp2.value().as_string();
				else if (kvp2.key() == "9slice_margins")
				{
					margins = boost::json::value_to<std::vector<int32_t>>(kvp2.as_object());
				}
			}
			
			tex::ImageData image;
			if(!tex::ReadTGA(path, &image))
				continue;
			
			group.addImage(texture_name, image, margins[0], margins[1], margins[2], margins[3]);
		}
	}
}
