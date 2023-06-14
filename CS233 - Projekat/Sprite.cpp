#include "Sprite.h"
#include "SOIL2.h"

Sprite::Sprite(const unsigned int p_numberOfTextures)
{
	m_textures = new GLuint[p_numberOfTextures];
	m_textureIndex = 0;
	m_currentFrame = 0;
	m_numberOfFrames = 0;
	m_animationDelay = 0.25f;
	m_animationElapsed = 0.0f;
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_size.height = 0.0f;
	m_size.width = 0.0f;
	m_collision.left = 0.0f;
	m_collision.right = 0.0f;
	m_collision.top = 0.0f;
	m_collision.bottom = 0.0f;

	m_velocity = 0.0f;
	m_radius = 0.0f;

	m_value = 0;

	m_isCollideable = false;
	m_flipHorizontal = false;
	m_flipVertical = false;
	m_isVisible = false;
	m_isActive = false;
	m_isSpriteSheet = false;
	m_isClicked = false;
}

Sprite::~Sprite()
{
	for (int i = 0; i < m_textureIndex; i++)
	{
		glDeleteTextures(1, &m_textures[i]);
	}
	delete[] m_textures;
	m_textures = NULL;
}

void Sprite::Update(const float p_deltaTime)
{
	float dt = p_deltaTime;
	if (m_isActive)
	{
		m_animationElapsed += dt;
		if (m_animationElapsed >= m_animationDelay)
		{
			m_currentFrame++;
			if (m_currentFrame >= m_numberOfFrames) m_currentFrame = 0;
			m_animationElapsed = 0.0f;
		}
		m_position.x += m_velocity * dt;
	}
}

void Sprite::Render()
{
	if (m_isVisible)
	{
		if (m_useTransparency)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBindTexture(GL_TEXTURE_2D, GetCurrentFrame());

		glBegin(GL_QUADS);

		GLfloat x = m_position.x;
		GLfloat y = m_position.y;

		GLfloat w = m_size.width;
		GLfloat h = m_size.height;

		GLfloat texWidth = (GLfloat)m_textureIndex / (GLfloat)m_numberOfFrames;
		GLfloat texHeight = 1.0f;
		GLfloat u = 0.0f;
		GLfloat v = 0.0f;
		if (m_textureIndex < m_numberOfFrames) u = (GLfloat)m_currentFrame * texWidth;

		glTexCoord2f(u, v); glVertex2f(x, y);
		glTexCoord2f(u + texWidth, v); glVertex2f(x + w, y);
		glTexCoord2f(u + texWidth, v + texHeight); glVertex2f(x + w, y + h);
		glTexCoord2f(u, v + texHeight); glVertex2f(x, y + h);

		glEnd();

		if (m_useTransparency)
		{
			glDisable(GL_BLEND);
		}
	}
}

const bool Sprite::AddTexture(const char* p_imageName, const bool p_useTransparency)
{
	GLuint texture = SOIL_load_OGL_texture( p_imageName, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0 );
	if (texture == 0)
	{
		return false;
	}

	m_textures[m_textureIndex] = texture;
	m_textureIndex++;
	if (m_textureIndex == 1 && m_numberOfFrames > 1)
	{
		m_isSpriteSheet= true;
	}
	else
	{
		m_isSpriteSheet = false;
	}
	m_useTransparency = p_useTransparency;
	return true;
}

void Sprite::Jump(SpriteState p_state)
{
	if (p_state == SpriteState::DOWN )
	{
		if (m_position.y < 470.0f) m_position.y += 75.0f;
	}
	else if (p_state == SpriteState::UP)
	{
		if (m_position.y >= 470.0f) m_position.y -= 75.0f;
	}
}

const Sprite::Rect Sprite::GetCollisionRect() const
{
	Rect rect;
	rect.left = m_position.x + m_collision.left;
	rect.right = m_position.x + m_size.width + m_collision.right;
	rect.top = m_position.y + m_collision.top;
	rect.bottom = m_position.y + m_size.height + m_collision.bottom;

	return rect; 
}

const bool Sprite::IntersectsRect(const Sprite* p_sprite) const
{
	if (this->IsCollideable() && p_sprite->IsCollideable() && this->IsActive() && p_sprite->IsActive())
	{
		const Rect recta = this->GetCollisionRect();
		const Rect rectb = p_sprite->GetCollisionRect();

		if (recta.left >= rectb.left && recta.left <= rectb.right &&
			recta.top >= rectb.top && recta.top <= rectb.bottom)
		{
			return true;
		}
		else if (recta.right >= rectb.left && recta.right <= rectb.right &&
			recta.top >= rectb.top && recta.top <= rectb.bottom)
		{
			return true;
		}
		else if (recta.left >= rectb.left && recta.right <= rectb.right &&
			recta.top < rectb.top && recta.bottom > rectb.bottom)
		{
			return true;
		}
		else if (recta.top >= rectb.top && recta.bottom <= rectb.bottom &&
			recta.left < rectb.left && recta.right > rectb.right)
		{
			return true;
		}
		else if (rectb.left >= recta.left && rectb.left <= recta.right &&
			rectb.top >= recta.top && rectb.top <= recta.bottom)
		{
			return true;
		}
		else if (rectb.right >= recta.left && rectb.right <= recta.right &&
			rectb.top >= recta.top && rectb.top <= recta.bottom)
		{
			return true;
		}
		else if (rectb.left >= recta.left && rectb.right <= recta.right &&
			rectb.top < recta.top && rectb.bottom > recta.bottom)
		{
			return true;
		}
		else if (recta.top >= rectb.top && recta.bottom <= rectb.bottom &&
			recta.left < rectb.left && recta.right > rectb.right)
		{
			return true;
		}
		else if (rectb.top >= recta.top && rectb.bottom <= recta.bottom &&
			rectb.left < recta.left && rectb.right > recta.right)
		{
			return true;
		}
	}

	return false;
}

const Sprite::Point Sprite::GetCenter() const
{
	Point center;
	center.x = this->GetPosition().x + m_center.x;
	center.y = this->GetPosition().y + m_center.y;

	return center;
}

const bool Sprite::IntersectsCircle(const  Sprite* p_sprite) const
{
	if (this->IsCollideable() && p_sprite->IsCollideable() && this->IsActive() && p_sprite->IsActive())
	{
		const Point p1 = this->GetCenter();
		const Point p2 = p_sprite->GetCenter();

		float y = p2.y - p1.y;
		float x = p2.x - p1.x;
		float d = x*x + y*y;

		float r1 = this->GetRadius() * this->GetRadius();
		float r2 = p_sprite->GetRadius() * p_sprite->GetRadius();

		if (d <= r1 + r2)
		{
			return true;
		}
	}
	return false;

}