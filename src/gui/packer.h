#ifndef PACKER_H
#define PACKER_H

#include "widget.h"
#include <functional>

class UIWindow;

class Packer
{
public:
    virtual ~Packer() = default;

    void setHorizontalSpacing(float val) { m_horizontal_spacing = val; }
    void setVerticalSpacing(float val) { m_vertical_spacing = val; }

    glm::vec2 getWidgetSize(Widget const & w, std::function<glm::vec2(Widget const &)> func) const;

    virtual void fitWidgets(UIWindow * win) const = 0;

protected:
    float m_horizontal_spacing = 1.0f;
    float m_vertical_spacing   = 1.0f;
};

class MatrixPacker : public Packer
{
public:
    using WidgetMatrix = std::vector<std::vector<Widget *>>;

    void fitWidgets(UIWindow * win) const override;

protected:
    WidgetMatrix getMatrixFromTree(Widget * root) const;
    void         addWidgetPtr(WidgetMatrix & mtx, Widget * ptr, int32_t x, int32_t y) const;
    void         addSubTree(WidgetMatrix & ls, Widget * root, int32_t x, int32_t y) const;
    float        getRowSumWidth(std::vector<Widget *> const & row) const;
    float        getSumOfFixedWidthInRow(std::vector<Widget *> const & row) const;
    int32_t      getNumOfScaledElementsInRow(std::vector<Widget *> const & row) const;
    float        getRowMaxHeight(std::vector<Widget *> const & row) const;
    void         adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const;
};

class TreePacker : public Packer
{
public:
    void fitWidgets(UIWindow * win) const override;

protected:
    struct GrupNodeProp
    {
        bool      is_horizontal           = true;
        bool      is_scalable             = false;
        int32_t   num_children            = 0;
        int32_t   num_fixed_size_elements = 0;
        glm::vec2 size                    = {0.f, 0.f};
        glm::vec2 size_hint               = {0.f, 0.f};
        glm::vec2 fixed_elements_size     = {0.f, 0.f};
    };

    void arrangeWidgetsInRow(Widget & row_node, glm::vec2 cur_tlpos, glm::vec2 const & win_size) const;
    void arrangeWidgetsInColumn(Widget & column_node, glm::vec2 cur_tlpos, glm::vec2 const & win_size) const;

    bool         isGroupNodeScalable(Widget const & node) const;
    GrupNodeProp getGroupNodeProperties(Widget const & node) const;
	void   placeWidgetInCell(Widget &  w, glm::vec2 top_left_pos, glm::vec2 scaled_size) const;
};

#endif
