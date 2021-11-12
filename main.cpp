

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

using namespace juce;

struct Editor : Component
{
	Editor()
	{
	}

	void resized() override
	{
		
	}
};

struct UltraEditorApp : JUCEApplication
{
	UltraEditorApp() = default;

	const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
	const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
	bool moreThanOneInstanceAllowed() override { return false; }

	//==============================================================================
	void initialise(const juce::String& commandLine) override
	{
	}

	void shutdown() override
	{
	}

	//==============================================================================
	void systemRequestedQuit() override
	{
		quit();
	}

	void anotherInstanceStarted(const juce::String &commandLine) override
	{
	}
};

START_JUCE_APPLICATION(UltraEditorApp);
