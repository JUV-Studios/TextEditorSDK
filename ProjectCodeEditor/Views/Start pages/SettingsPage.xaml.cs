﻿using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace ProjectCodeEditor.Views.Start_pages
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SettingsPage : Page
    {
        public SettingsPage() => InitializeComponent();

        private void FontBox_TextSubmitted(ComboBox sender, ComboBoxTextSubmittedEventArgs args)
        {

        }
    }
}
