#include "pch.h"
#include "CodeEditor.h"
#if __has_include("CodeEditor.g.cpp")
#include "CodeEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::UI::Text;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::Develop::implementation
{
	CodeEditor::CodeEditor(StorageFile const& file)
	{
		WorkingFile(file);
		Saved(true);
		TextView(TextEditor::UI::SyntaxEditor());
		InitializeComponent();
	}

	void CodeEditor::AfterUndoRedoInvoked(bool lastElem, hstring const& text, int index)
	{
		TextView().Focus(FocusState::Keyboard);
		if (lastElem)
		{
			TextView().Text(m_FileReadData->Text);
			TextView().TextDocument().Selection().StartPosition(0);
		}
		else
		{
			TextView().Text(text);
			TextView().TextDocument().Selection().StartPosition(index);
			TextView().TextDocument().Selection().EndPosition(index - 1);
			TextView().TextDocument().Selection().Collapse(false);
		}
	}

	bool CodeEditor::TextChanged(hstring const& text, RoutedEventArgs const&)
	{
		if (m_FileReadData)
		{
			auto valTrim = JUVStudios::Helpers::TrimEnd(text, JUVStudios::Helpers::SpaceCharacters());
			if (valTrim == JUVStudios::Helpers::TrimEnd(m_FileReadData->Text, JUVStudios::Helpers::SpaceCharacters()))
			{
				SaveFileAsync();
				TextView().ClearHistory();
				return false;
			}
			else
			{
				if (TextView().CanUndo())
				{
					if (valTrim != TextView().PreviousText()) goto invalidateState;
				}
				else goto invalidateState;
			}
		}

		return false;
	invalidateState:
		if (SettingsViewModel::Instance().AutoSave()) SaveFileAsync();
		else Saved(false);
		return true;
	}

	fire_and_forget CodeEditor::LoadFileAsync()
	{
		if (!m_LoadSaveLock)
		{
			m_LoadSaveLock = true;
			apartment_context uiThread;
			bool isRichText = WorkingFile().FileType() == L".rtf" || WorkingFile().FileType() == L".RTF";
			TextView().Initialize(isRichText);
			if (isRichText)
			{
				auto stream = co_await WorkingFile().OpenReadAsync();
				TextView().TextDocument().LoadFromStream(TextSetOptions::FormatRtf, stream);
			}
			else
			{
				co_await resume_background();
				auto readBuffer = co_await FileIO::ReadBufferAsync(WorkingFile());
				m_FileReadData = JUVStudios::EncodingDetect::DetectPlusGet(readBuffer);
				co_await uiThread;
				TextView().HistoryDone({ this, &CodeEditor::AfterUndoRedoInvoked });
				TextView().Text(m_FileReadData->Text);
			}

			Saved(true);
			TextView().TextChangeDelegate({ this, &CodeEditor::TextChanged });
			Ring().IsActive(false);
			Content().Visibility(Visibility::Visible);
			m_LoadSaveLock = false;
		}
	}

	IAsyncAction CodeEditor::SaveFileAsync()
	{
		if (!m_FileReadData) return;
		if (!m_LoadSaveLock)
		{
			m_LoadSaveLock = true;
			if (TextView().IsRichText())
			{
				CachedFileManager::DeferUpdates(WorkingFile());
				auto stream = co_await WorkingFile().OpenAsync(FileAccessMode::ReadWrite);
				TextView().TextDocument().SaveToStream(TextGetOptions::FormatRtf, stream);
				co_await CachedFileManager::CompleteUpdatesAsync(WorkingFile());
			}
			else co_await FileIO::WriteTextAsync(WorkingFile(), TextView().Text(), m_FileReadData->Encoding);
			Saved(true);
			m_LoadSaveLock = false;
		}
	}

	IAsyncAction CodeEditor::CloseAsync() 
	{
		co_await SaveFileAsync();
		Close();
	}

	void CodeEditor::Close()
	{
		TextView().Close();
		TextView(nullptr)
		m_Bindable = nullptr;
		m_FileReadData.reset();
	}

	bool CodeEditor::PrepareClose() 
	{
		if (!Saved())
		{
			return false;
		}
		else
		{
			Bindings->StopTracking();
			return true;
		}
	}

	void CodeEditor::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
	{
		if (Ring().IsActive()) LoadFileAsync();
	}

	void CodeEditor::SaveFile_Click(IInspectable const&, RoutedEventArgs const&) { SaveFileAsync(); }
}