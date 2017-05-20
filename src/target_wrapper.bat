@echo off
SetLocal EnableDelayedExpansion
(set PATH=C:\Qt5.8\5.8\msvc2015_64\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=C:\Qt5.8\5.8\msvc2015_64\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=C:\Qt5.8\5.8\msvc2015_64\plugins
)
%*
EndLocal
