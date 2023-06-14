#pragma once
#include <stdlib.h>
#include <GL/glut.h>

class Sprite
{
public:
	struct Point
	{
		GLfloat x;
		GLfloat y;
	};

	struct Size
	{
		GLfloat width;
		GLfloat height;
	};

	struct Rect
	{
		GLfloat top;
		GLfloat bottom;
		GLfloat left;
		GLfloat right;
	};

	enum SpriteState
	{
		UP,
		DOWN
	};

protected:
	GLuint* m_textures;
	unsigned int m_textureIndex;
	unsigned int m_currentFrame;
	unsigned int m_numberOfFrames;
	GLfloat m_animationDelay;
	GLfloat m_animationElapsed;

	Point m_position;
	Size m_size;
	GLfloat m_velocity;

	GLfloat m_radius;
	Point m_center;

	int m_value;

	Rect m_collision;

	bool m_isCollideable;
	bool m_flipHorizontal;
	bool m_flipVertical;
	bool m_isVisible;
	bool m_isActive;
	bool m_useTransparency;
	bool m_isSpriteSheet;
	bool m_isClicked;

public:
	Sprite(const unsigned int p_numberOfTextures);
	~Sprite();

	void Update(const float p_deltaTime);
	void Render();

	const bool AddTexture(const char* p_fileName, const bool p_useTransparency = true);
	const GLuint GetCurrentFrame() const { return m_isSpriteSheet ? m_textures[0] : m_textures[m_currentFrame]; }

	void SetPosition(const GLfloat p_x, const GLfloat p_y) { m_position.x = p_x; m_position.y = p_y; }
	void SetPosition(const Point p_position) { m_position.x = p_position.x; m_position.y = p_position.y; }
	void SetFrameSize(const GLfloat p_width, const GLfloat p_height) { m_size.width = p_width; m_size.height = p_height; }
	void SetVelocity(const GLfloat p_velocity) { m_velocity = p_velocity; }
	void SetNumberOfFrames(const unsigned int p_frames) { m_numberOfFrames = p_frames; }

	const  GLfloat GetVelocity() const { return m_velocity; }

	const Point GetPosition() const { return m_position; }
	const Size GetSize() const { return m_size; }
	void IsCollideable(const bool p_value) { m_isCollideable = p_value; }
	const bool IsCollideable() const { return m_isCollideable; }
	void FlipHorizontal(const bool p_value) { m_flipHorizontal = p_value; }
	void FlipVertical(const bool p_value) { m_flipVertical = p_value; }
	void IsActive(const bool p_value) { m_isActive = p_value; }
	const bool IsActive() const { return m_isActive; }
	void IsVisible(const bool p_value) { m_isVisible = p_value; }
	const bool IsVisible() const { return m_isVisible; }
	void UseTransparency(const bool p_value) { m_useTransparency = p_value; }
	
	void IsClicked(const bool p_value) { m_isClicked = p_value; }
	const bool IsClicked() const { return m_isClicked; }
	void Jump(SpriteState p_state);

	const Rect GetCollisionRect() const;
	void SetCollisionRectOffset(const Rect p_rect) { m_collision = p_rect; }
	const bool IntersectsRect(const Sprite* p_sprite) const;

	void SetRadius(const GLfloat p_radius) { m_radius = p_radius; }
	const GLfloat GetRadius() const { return m_radius; }
	void SetCenter(const Point p_center) { m_center = p_center; }
	const Point GetCenter() const;
	const bool IntersectsCircle(const Sprite* p_sprite) const;

	void SetValue(const int p_value) { m_value = p_value; }
	const int GetValue() const { return m_value; }
};

