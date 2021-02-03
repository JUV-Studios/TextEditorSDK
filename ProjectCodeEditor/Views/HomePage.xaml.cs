﻿using Microsoft.Toolkit.Uwp.Extensions;
using ProjectCodeEditor.Core.Helpers;
using ProjectCodeEditor.Dialogs;
using ProjectCodeEditor.Helpers;
using ProjectCodeEditor.Models;
using ProjectCodeEditor.ViewModels;
using Windows.ApplicationModel.DataTransfer;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Automation;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Documents;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace ProjectCodeEditor.Views
{
    public sealed partial class HomePage : UserControl
    {
        public readonly HomeViewModel ViewModel = new HomeViewModel();

        public readonly string SettingsTitleStringId = "SettingsHub/Header";

        public HomePage() => InitializeComponent();

        private void RecentFilesListItem_Click(object sender, ItemClickEventArgs e) => Interactions.AddStorageItems(new IStorageItem2[] { (e.ClickedItem as RecentItem).Item });

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            DataTransferManager.GetForCurrentView().DataRequested += HomePage_DataRequested;
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            DataTransferManager.GetForCurrentView().DataRequested -= HomePage_DataRequested;
        }

        private void HomePage_DataRequested(DataTransferManager sender, DataRequestedEventArgs args)
        {
            var deferral = args.Request.GetDeferral();
            if (ViewModel.ContextedRecentFile != null)
            {
                args.Request.Data.Properties.Title = string.Format("ShareFileTitle".GetLocalized(), ViewModel.ContextedRecentFile.Item.Name);
                args.Request.Data.Properties.Description = string.Format("ShareFileCaption".GetLocalized(), ViewModel.ContextedRecentFile.Item.Name);
                args.Request.Data.SetStorageItems(new IStorageItem[] { ViewModel.ContextedRecentFile.Item });
            }

            deferral.Complete();
        }

        private void Layout_Loaded(object sender, RoutedEventArgs e)
        {
            var layout = sender as Hub;
            AccessibilityHelper.SetProperties(layout);
            layout.Loaded -= Layout_Loaded;
        }

        private void FontBox_TextSubmitted(ComboBox sender, ComboBoxTextSubmittedEventArgs args)
        {
            args.Handled = true;
            string input = args.Text.Trim();
            // var contains = App.AppSettings.InstalledFonts.Where(i => i.Trim() == input.Trim());
            // if (!contains.IsEmpty()) sender.SelectedValue = args.Text.Trim();
        }

        private void SpecialThanksBlock_Loaded(object sender, RoutedEventArgs e)
        {
            var specialThanksBlock = sender as TextBlock;
            var link = new Hyperlink();
            link.Inlines.Add(new Run() { Text = "Adnan Umer" });
            link.NavigateUri = new("mailto:aztnan@outlook.com");
            AutomationProperties.SetName(link, $"{"SpecialThanksBlock/Text".GetLocalized()} Adnan Umer");
            specialThanksBlock.Inlines.Add(new Run() { Text = $"{"SpecialThanksBlock/Text".GetLocalized()} " });
            specialThanksBlock.Inlines.Add(link);
            specialThanksBlock.Loaded -= SpecialThanksBlock_Loaded;
        }

        private void DependenciesDialog_Click(object sender, RoutedEventArgs e) => DialogHelper.ShowPlusBlock(Singleton<DependenciesDialog>.Instance, null);

        private void RecentsListViewSearch_Submit(AutoSuggestBox sender, AutoSuggestBoxQuerySubmittedEventArgs args) => Singleton<RecentFilesList>.Instance.Search(args.QueryText);

        // private void RecentFileContextMenu_Opening(object sender, object e) => ClearFilesOption.IsEnabled = Singleton<RecentFilesList>.Instance.ItemsSource == RecentsViewModel.RecentFiles;
    }
}