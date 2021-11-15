

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "cpu.h"
#include "memory.h"
#include "magic_enum.hpp"
#include "disassembler.h"

#include <cstring>

#define for_each(container, elem_name, ...) for (auto& e : container) __VA_ARGS__

using namespace juce;

// Implemented in parser.cpp
const char* parser_get_symbolic_gpr_name(int i);
const char* parser_get_symbolic_cop0_name(int i);

static void layout_items(bool horizontally, std::initializer_list<Component*>&& items, Rectangle<int> bounds)
{
	if (horizontally)
	{
		const auto item_size = bounds.getWidth() / items.size();

		for (auto* c : items)
			c->setBounds(bounds.removeFromLeft(item_size));
	}
	else
	{
		const auto item_size = bounds.getHeight() / items.size();

		for (auto* c : items)
			c->setBounds(bounds.removeFromTop(item_size));
	}
}

template<typename T>
struct LabelledContainer : Component
{
	T content;
	Label label;

	template<typename... Args>
	LabelledContainer(const String& text, Args&&... args) :
		content(std::forward<Args>(args)...),
		label({}, text)
	{
		addAndMakeVisible(content);
		addAndMakeVisible(label);

		label.setFont(Font(18));
	}

	T* get() { return &content; }

	void resized() override
	{
		auto bounds = getLocalBounds();

		label.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.16));
		content.setBounds(bounds);
	}
};

struct RegisterEditor : Label
{
	void* register_ref;
	uint64_t previous_value;
	int type_size;

	RegisterEditor(void* register_pointer, int width) :
		register_ref(register_pointer),
		type_size(width)
	{
		onTextChange = [this]
		{
			if (type_size == 4)
				*(uint32_t*)register_ref = (uint32_t)getText().getHexValue32();
			else
				*(uint64_t*)register_ref = (uint64_t)getText().getHexValue64();
		};

		setFont(Font(18));
		setEditable(true, true, true);

		update();
	}

	void update()
	{
		if (memcmp(&previous_value, register_ref, type_size) != 0)
		{
			// mark as dirty
			setColour(Label::ColourIds::outlineColourId, { 255, 0, 0 });
		}
		else
		{
			setColour(Label::ColourIds::outlineColourId, Colours::transparentBlack);
		}

		if (type_size == 4)
			setText(String::formatted("%08X", *(uint32_t*)register_ref), dontSendNotification);
		else
			setText(String::formatted("%016llX", *(uint64_t*)register_ref), dontSendNotification);
		
		memcpy(&previous_value, register_ref, type_size);
	}
};

struct RegisterPanel : Component
{
	LabelledContainer<RegisterEditor>* pc_editor;

	std::vector<LabelledContainer<RegisterEditor>*> editors;

	RegisterPanel()
	{
		pc_editor = new LabelledContainer<RegisterEditor>("pc", &cpu_get_pc_register(), 8);

		for (int i = 0; i < 32; i++)
		{
			editors.push_back(new LabelledContainer<RegisterEditor>(
				parser_get_symbolic_gpr_name(i), &cpu_get_gp_register(i), 8
			));
		}

		for (int i = 0; i < 32; i++)
		{
			editors.push_back(new LabelledContainer<RegisterEditor>(
				parser_get_symbolic_cop0_name(i), &cpu_get_cp0_register(i), 8
			));
		}

		editors.push_back(new LabelledContainer<RegisterEditor>("lo", &cpu_get_lo_register(), 4));
		editors.push_back(new LabelledContainer<RegisterEditor>("hi", &cpu_get_hi_register(), 4));
		
		for (auto* editor : editors)
			addAndMakeVisible(editor);

		addAndMakeVisible(*pc_editor);
	}

	void update()
	{
		pc_editor->get()->update();
		for_each(editors, e, e->get()->update());
	}

	void resized() override
	{
		auto bounds = getLocalBounds();

		pc_editor->setBounds(bounds.removeFromTop(30));

		auto num_editors = editors.size();
		auto h = bounds.getHeight() / (num_editors / 2);
		auto l_bounds = bounds.removeFromLeft(bounds.getWidth() / 2);
		auto r_bounds = bounds;

		for (int i = 0; i < num_editors; i += 2)
		{
			editors[i + 0]->setBounds(l_bounds.removeFromTop(h));
			editors[i + 1]->setBounds(r_bounds.removeFromTop(h));
		}
	}
};

template<size_t Capacity>
struct TraceBuffer
{
	uint64_t entries[Capacity]{};

	auto begin() const { return entries + 0; }
	auto end() const { return entries + Capacity; }

	void add(uint64_t address)
	{
		for (int i = 0; i < Capacity - 1; i++)
			entries[i] = entries[i + 1];

		entries[Capacity - 1] = address;
	}
};

struct Debugger : Thread
{
	enum class State : int { Paused, Running, Step };
	std::vector<uint64_t> breakpoints;
	std::atomic<State> state{};

	TraceBuffer<32> trace_buffer;

	Debugger() : 
		Thread("DebuggerThread")
	{
		startThread();
	}

	~Debugger()
	{
		stopThread(1000);
	}

	std::function<void()> on_step = []{};

	static Debugger* instance()
	{
		static Debugger debugger;
		return &debugger;
	}

	State get_state() const { return state; }

	void add_breakpoint(uint64_t address)
	{
		breakpoints.push_back(address);
	}

	void remove_breakpoint(uint64_t address)
	{
		std::remove(breakpoints.begin(), breakpoints.end(), address);
	}

	void run_cpu()
	{
		state = State::Running;
		notify();
	}

	void stop_cpu()
	{
		state = State::Paused;
		notify();
	}

	void step_cpu()
	{
		state = State::Step;
		notify();
	}

	void run() override
	{
		cpu_init();

		while (!threadShouldExit())
		{
			switch (state)
			{
				case State::Paused:
				{
					// do nothing
					wait(100);
				} break;
				
				case State::Running: 
				{
					if (!step())
					{
						state = State::Paused;
					}
				} break;
				
				case State::Step:
				{
					step();
					state = State::Paused;
					notify();
				} break;
			}
		}
	}

private:
	bool step()
	{
		auto pc = cpu_get_pc_register();

		trace_buffer.add(pc);

		if (cpu_step())
		{
			on_step();
			
			return true;
		}

		return false;
	}
};

struct Disassembler : Component
{
	struct InstructionCacheEntry
	{
		uint64_t address;
		uint32_t hit_counter;
		uint32_t opcode;
		std::array<char, 2024> string;
	};

	int line_height{25};

	std::vector<InstructionCacheEntry> cache;
	
	Disassembler()
	{
		setOpaque(true);
	}

	const InstructionCacheEntry& get_instruction(uint64_t address)
	{
		auto& item = [this](uint64_t address)->InstructionCacheEntry&
		{
			for (auto& item : cache)
			{
				if (item.address == address)
					return item;
			}

			cache.push_back({ address, 0, 0, {} });
			return cache[cache.size()-1];
		}(address);

		if (item.hit_counter == 0)
		{
			uint32_t opcode;
			
			if (memory_read32(address, opcode))
			{
				if (auto* desc = disassembler_decode_instruction(opcode))
				{
					disassembler_parse_instruction(opcode, desc, item.string.data(), 0);
					item.opcode = opcode;
					item.address = address;
				}
			}
		}

		item.hit_counter++;
		return item;
	}

	void paint(Graphics& g) override
	{
		auto bounds = getLocalBounds().toFloat().reduced(0, 10);

		const auto num_lines = bounds.getHeight() / line_height;

		g.fillAll({25, 25, 25});

		g.setColour({255, 255, 255});
		g.setFont(Font("Courier New", line_height * 0.8, 0));

		const auto current_address = cpu_get_next_instruction_address();
		uint64_t address = current_address - 0x20;
		for (int i = 0; i < num_lines; i++)
		{
			const auto& inst = get_instruction(address);

			auto text_bounds = bounds.removeFromTop(line_height);

			if (address == current_address)
			{
				g.saveState();

				g.setColour({ 255, 0, 255 });

				auto b = text_bounds;
				g.drawText("-->", b.removeFromLeft(50), Justification::centredRight);

				g.restoreState();
			}

			g.drawText(
				String::formatted("0x%016llX: %08X: %s", address, inst.opcode, inst.string),
				text_bounds.withTrimmedLeft(50),
				Justification::centredLeft,
				false
			);

			address += 4;
		}
	}

	void update()
	{
		repaint();
	}

	void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override
	{
	}
};

struct Editor : Component, Timer
{
	TextButton play_pause_button;
	TextButton step_button;
	RegisterPanel register_panel;
	Disassembler disassembler;

	Editor()
	{
		play_pause_button.setButtonText(magic_enum::enum_name(Debugger::instance()->get_state()).data());
		step_button.setButtonText("step");

		play_pause_button.setClickingTogglesState(true);
		play_pause_button.onClick = [this]
		{
			if (play_pause_button.getToggleState())
			{
				Debugger::instance()->run_cpu();
				startTimerHz(30);
			}
			else
			{
				Debugger::instance()->stop_cpu();
				stopTimer();
				update();
			}

			play_pause_button.setButtonText(magic_enum::enum_name(Debugger::instance()->get_state()).data());
		};
		
		step_button.onClick = [this]
		{
			Debugger::instance()->step_cpu();
			update();
		};

		addAndMakeVisible(play_pause_button);
		addAndMakeVisible(step_button);
		addAndMakeVisible(register_panel);
		addAndMakeVisible(disassembler);

		Debugger::instance()->on_step = [this]
		{
			if (Debugger::instance()->get_state() != Debugger::State::Running)
			{
				Timer::callAfterDelay(0, [this]{ update(); });
			}
		};
	}

	void timerCallback() override
	{
		switch (Debugger::instance()->get_state())
		{
			case Debugger::State::Running:
			{
				update();
			} break;
			
			case Debugger::State::Paused:
			{
			} break;
		}
	}

	void update()
	{
		register_panel.update();
		disassembler.update();
	}

	void resized() override
	{
		auto bounds = getLocalBounds();
		auto panel_bounds = bounds.removeFromRight(500);

		layout_items(true, { &play_pause_button, &step_button }, panel_bounds.removeFromTop(35));

		register_panel.setBounds(panel_bounds);

		disassembler.setBounds(bounds);
	}
};

void cpu_run_tests();

struct UltraEditorApp : JUCEApplication
{
	DocumentWindow* window{};

	UltraEditorApp() = default;

	const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
	const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
	bool moreThanOneInstanceAllowed() override { return false; }

	//==============================================================================
	void initialise(const juce::String& commandLine) override
	{
		cpu_run_tests();
		
		// init the debugger
		// all cpu ops are done on the debugger thread
		//Debugger::instance();

		//window = new DocumentWindow("ultra", { 0, 0, 0 }, DocumentWindow::allButtons);
		//window->setContentOwned(new Editor, false);
		//window->setUsingNativeTitleBar(true);
		//window->setResizable(true, false);
		//window->centreWithSize(1200, 800);
		//window->setVisible(true);
	}

	void shutdown() override
	{
		delete window;
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
