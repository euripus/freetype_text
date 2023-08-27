#ifndef WIDGET_H
#define WIDGET_H

#include <vector>

class Widget
{
public:
	Widget() = default;
	virtual ~Widget();
	
	virtual void update(float time);
	virtual void draw();

	Widget * parent() const;

	void show();					
	void hide();				
	bool visible() const;

	glm::vec2 size() const;
	
	glm::vec2 pos() const;
	void move(const glm::vec2 & point);
	void resize(const glm::vec2 & new_size);

protected:
	glm::vec2 m_size;
	glm::vec2 m_size_min;
	glm::vec2 m_size_desired;
	glm::vec2 m_pos;
	
	bool m_visible;

	Widget * parent;
	std::vector<std::unique_ptr<Widget>> m_children;
	
	friend Packer;
};

#endif