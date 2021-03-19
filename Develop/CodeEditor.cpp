#include "CodeEditor.h"
#if __has_include("CodeEditor.g.cpp")
#include "CodeEditor.g.cpp"
#endif

using namespace winrt;
using namespace Shared;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Text;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::ApplicationModel::Core;

namespace winrt::Develop::implementation
{
	CodeEditor::CodeEditor(StorageFile const& file) : m_WorkingFile(file)
	{
		InitializeComponent();
	}

	fire_and_forget CodeEditor::LoadFileAsync()
	{
		co_await TextView().LoadAsync(m_WorkingFile);
		if (!m_EventRegistrations)
		{
			m_EventRegistrations.emplace();
			m_EventRegistrations->first = CoreApplication::GetCurrentView().CoreWindow().Dispatcher().AcceleratorKeyActivated({ this, &CodeEditor::KeyPressHandler });
			m_EventRegistrations->second = TextView().PropertyChanged({ this, &CodeEditor::EditorPropertyChanged });
		}
	}

	bool CodeEditor::Saved() const
	{
		return m_Saved;
	}

	void CodeEditor::Saved(bool value)
	{
		if (m_Saved != value)
		{
			m_Saved = value;
			m_PropertyChanged(*this, PropertyChangedEventArgs(L"Saved"));
		}
	}

	StorageFile CodeEditor::WorkingFile()
	{
		return m_WorkingFile;
	}

	bool CodeEditor::StartClosing()
	{
		Bindings->StopTracking();
		return true;
	}

	IAsyncAction CodeEditor::CloseAsync()
	{
		if (!m_Saved) co_await SaveFile_Click(nullptr, nullptr);
		Close();
	}

	void CodeEditor::Close()
	{
		// Remove event handlers
		if (m_EventRegistrations)
		{
			CoreApplication::GetCurrentView().CoreWindow().Dispatcher().AcceleratorKeyActivated(m_EventRegistrations->first);
			TextView().PropertyChanged(m_EventRegistrations->second);
			m_EventRegistrations.reset();
		}

		TextView().Close();
	}

	void CodeEditor::KeyPressHandler(Windows::UI::Core::CoreDispatcher const&, AcceleratorKeyEventArgs const& e)
	{
		if (!AppSettings::DialogShown() && !m_Unloaded && e.EventType() == CoreAcceleratorKeyEventType::KeyDown &&
			(CoreApplication::GetCurrentView().CoreWindow().GetKeyState(VirtualKey::Control) & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
		{
			switch (e.VirtualKey())
			{
			case VirtualKey::Z:
				// Handle Ctrl + Z
				e.Handled(true);
				TextView().Undo();
				break;
				
			case VirtualKey::Y:
				// Handle Ctrl + Y
				e.Handled(true);
				TextView().Redo();
				break;

			case VirtualKey::S:
				// Handle Ctrl + S
				e.Handled(true);
				SaveFile_Click(nullptr, nullptr);
				break;

			default:
				// Don't handle the event
				e.Handled(false);
				break;
			}
		}
	}

	void CodeEditor::EditorPropertyChanged(IInspectable const&, PropertyChangedEventArgs const& args)
	{
		auto request = args.PropertyName();
		if (request == L"Client_SaveRequested") SaveFile_Click(nullptr, nullptr);
		else if (request == L"Client_InvalidateSaveState") Saved(false);
	}

	void CodeEditor::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
	{
		m_Unloaded = false;
		if (!TextView().FileLoaded()) LoadFileAsync();
	}

	void CodeEditor::UserControl_Unloaded(IInspectable const&, RoutedEventArgs const&)
	{
		m_Unloaded = true;
	}

	void CodeEditor::StandardCommand_Loaded(IInspectable const& sender, RoutedEventArgs const&)
	{
		auto target = sender.as<AppBarButton>();
		auto tag = unbox_value<hstring>(target.Tag());
		StandardUICommand command;
		if (tag == L"Undo") command.Kind(StandardUICommandKind::Undo);
		else if (tag == L"Redo") command.Kind(StandardUICommandKind::Redo);
		else if (tag == L"SelectAll") command.Kind(StandardUICommandKind::SelectAll);
		else if (tag == L"Save") command.Kind(StandardUICommandKind::Save);
		if (target.Label().empty())
		{
			IconSourceElement iconElement;
			iconElement.IconSource(command.IconSource());
			target.Label(command.Label());
			target.Icon(iconElement);
			target.AccessKey(command.AccessKey());
			ToolTipService::SetToolTip(target, box_value(command.Description()));
		}
	}

	IAsyncAction CodeEditor::SaveFile_Click(IInspectable const&, RoutedEventArgs const&)
	{
		if (TextView().FileLoaded() && m_FileSaveLock.try_lock())
		{
			if (!co_await TextView().WriteFileAsync(m_WorkingFile))
			{
				// TODO Work on recovery system
			}
			else Saved(true);
			m_FileSaveLock.unlock();
		}
	}

	event_token CodeEditor::PropertyChanged(PropertyChangedEventHandler const& handler) noexcept
	{
		return m_PropertyChanged.add(handler);
	}

	void CodeEditor::PropertyChanged(event_token token) noexcept
	{
		m_PropertyChanged.remove(token);
	}

	void CodeEditor::EditorCommand_Requested(IInspectable const& sender, RoutedEventArgs const&)
	{
		if (!TextView().FileLoaded()) return;
		auto target = sender.as<AppBarButton>();
		auto tag = unbox_value<hstring>(target.Tag());
		if (tag == L"Undo") TextView().Undo();
		else if (tag == L"Redo") TextView().Redo();
		else if (tag == L"SelectAll") TextView().SelectAll();
		else if (tag == L"ClearSelection") TextView().ClearSelection();
	}

	CodeEditor::~CodeEditor()
	{
		// For tracking memory leaks
		OutputDebugString((L"Code editor for " + m_WorkingFile.Path() + L" has been destroyed\r").data());
	}
}
