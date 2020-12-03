﻿using Microsoft.AppCenter;
using Microsoft.AppCenter.Analytics;
using Microsoft.AppCenter.Crashes;
using ProjectCodeEditor.Core.Helpers;
using ProjectCodeEditor.Helpers;
using ProjectCodeEditor.Services;
using ProjectCodeEditor.ViewModels;
using ProjectCodeEditor.Views;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.ApplicationModel.Activation;
using Windows.Storage;
using Windows.System;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ProjectCodeEditor
{
    public sealed partial class App : Application
    {
        private readonly Stopwatch LaunchStopwatch = new();

        public App()
        {
            LaunchStopwatch.Start();
            InitializeComponent();
            ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.Maximized;
            AppCenter.Start("dd9a81de-fe79-4ab8-be96-8f96c346c88e", typeof(Analytics), typeof(Crashes));
            UnhandledException += OnAppUnhandledException;
        }

        protected override async void OnLaunched(LaunchActivatedEventArgs args) => await ActivateAsync(args);

        protected override async void OnActivated(IActivatedEventArgs args) => await ActivateAsync(args);

        protected override async void OnFileActivated(FileActivatedEventArgs args) => await ActivateAsync(args);

        private void OnAppUnhandledException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            // For more info see https://docs.microsoft.com/uwp/api/windows.ui.xaml.application.unhandledexception
        }

        public async Task ActivateAsync(object activationArgs)
        {
            Frame frame = null;
            if (IsInteractive(activationArgs))
            {
                // Initialize services that you need before app activation
                // take into account that the splash screen is shown while this code runs.
                await InitializeAsync();

                // Do not repeat app initialization when the Window already has content,
                // just ensure that the window is active
                if (Window.Current.Content == null)
                {
                    // Create a Shell or Frame to act as the navigation context
                    frame = new Frame();
                    Window.Current.Content = frame;
                    frame.NavigationFailed += (sender, e) => throw e.Exception;
                }
                else frame = Window.Current.Content as Frame;

                object arguments = null;
                if (activationArgs is LaunchActivatedEventArgs launchArgs) arguments = launchArgs.Arguments;

                frame.Navigate(typeof(MainPage), arguments);

                var activation = activationArgs as IActivatedEventArgs;

                // Ensure the current window is active
                Window.Current.Activate();

                if (activationArgs is FileActivatedEventArgs fileActivationArgs)
                {
                    ShellViewModel viewModel = Singleton<ShellViewModel>.Instance;
                    StorageFile[] filesArray = new StorageFile[fileActivationArgs.Files.Count];
                    for (int i = 0; i < fileActivationArgs.Files.Count; i++)
                    {
                        if (fileActivationArgs.Files[i].IsOfType(StorageItemTypes.File)) filesArray[i] = fileActivationArgs.Files[i] as StorageFile;
                    }

                    Interactions.AddFiles(filesArray);
                }
                else if (activationArgs is ProtocolActivatedEventArgs)
                {

                }

                LaunchStopwatch.Stop();
                Debug.WriteLine($"Develop took {LaunchStopwatch.ElapsedMilliseconds} ms to launch");
            }
        }

        private async Task InitializeAsync()
        {
            await Singleton<RecentsViewModel>.Instance.LoadRecentsAsync();
            await JumpListHelper.InitializeAsync();
            ViewService.Initialize();
        }

        private bool IsInteractive(object args) => args is IActivatedEventArgs;
    }
}

namespace System.Runtime.CompilerServices
{
    public class IsExternalInit { }
}
