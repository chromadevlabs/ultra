
#include "window.h"
#include "context.h"
#include "theme.h"

Window::Window(Component* content, const String& title, Point<int> size) : 
	DocumentWindow(title, {0, 0, 0}, allButtons)
{
	setContentOwned(content, false);
	setUsingNativeTitleBar(true);
	centreWithSize(size.x, size.y);
	setResizable(true, false);
	setVisible(true);
}

void Window::closeButtonPressed()
{
	context->windows.remove(this);

	if (onCloseButtonPressed)
	{
		onCloseButtonPressed();
	}
	else
	{
		if (context->windows.empty())
			JUCEApplication::quit();
	}

	delete this;
}

OpenGLWrapperComponent::OpenGLWrapperComponent(Component* contentComponent) :
	content(std::unique_ptr<Component>(contentComponent))
{
	setOpaque(true);
	setSize(512, 512);

	glContext.attachTo(*this);

	addAndMakeVisible(*content);
}

void OpenGLWrapperComponent::paint(Graphics& g)
{
	g.fillAll(theme->backgroundColour);
}

void OpenGLWrapperComponent::resized()
{
	content->setBounds(getLocalBounds());
}