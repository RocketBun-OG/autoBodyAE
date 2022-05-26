scriptname autoBodyMCM extends SKI_ConfigBase


autoBodyQuest Property MainScript Auto

GlobalVariable Property ABAEBackupPlayer Auto


event OnPageReset(string page)
    SetCursorFillMode(TOP_TO_BOTTOM)
    SetCursorPosition(0)
    
    if(page == "General Options" || page == "")
        AddHeaderOption("Feature Tweaks")
        AddToggleOptionST("BackupPlayer", "Use Raw Master For Player", ABAEBackupPlayer.GetValue() as int)
        
        AddHeaderOption("Keybinds")
        AddKeyMapOptionST("MenuBind", "In-Game Menu Keybind", MainScript.PresetKey)
        AddKeyMapOptionST("ModBind", "Modifier Keybind", MainScript.ModifierKey)

    endif

endEvent

State MenuBind
    Event OnHighlightST()
        SetInfoText("The key used to open the in-game preset menu.")
    endEvent

    Event OnInputOpenST()
        SetInputDialogStartText(MainScript.PresetKey)
    EndEvent

    ;unregister for the old key, then remap to the new one. 
    Event OnKeyMapChangeST(int keyCode, string conflictControl, string conflictName)
        MainScript.UnregisterForKey(MainScript.PresetKey)
       
        MainScript.PresetKey = keyCode
        SetKeyMapOptionValueST(MainScript.PresetKey)

        MainScript.RegisterForKey(MainScript.PresetKey)
    endEvent
EndState

State ModBind
    Event OnHighlightST()
        SetInfoText("The key used to modify autoBody commands to make them do something slightly different.")
    endEvent

    Event OnInputOpenST()
        SetInputDialogStartText(MainScript.ModifierKey)
    EndEvent

    ;unregister for the old key, then remap to the new one. 
    Event OnKeyMapChangeST(int keyCode, string conflictControl, string conflictName)
        MainScript.UnregisterForKey(MainScript.ModifierKey)
       
        MainScript.ModifierKey = keyCode
        SetKeyMapOptionValueST(MainScript.ModifierKey)

        MainScript.RegisterForKey(MainScript.ModifierKey)
    endEvent
EndState

State BackupPlayer
    Event OnHighlightST()
		SetInfoText("Use the true master preset pool for the player, instead of the one defined inside morphs.ini")
		
	EndEvent

    Event OnSelectST()
        if (ABAEBackupPlayer.GetValue() as int == 0)
            ABAEBackupPlayer.SetValue(1)
            SetToggleOptionValueST(1)
        else
            ABAEBackupPlayer.SetValue(0)
            SetToggleOptionValueST(0)
        endif
        
    endEvent
EndState