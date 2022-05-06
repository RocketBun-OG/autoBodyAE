scriptname autoBodyMCM extends SKI_ConfigBase


autoBodyQuest Property MainScript Auto

event OnPageReset(string page)
    SetCursorFillMode(TOP_TO_BOTTOM)
    SetCursorPosition(0)
    
    if(page == "General Options" || page == "")
        AddHeaderOption("Keybinds")
        AddKeyMapOptionST("MenuBind", "In-Game Menu Keybind", MainScript.PresetKey)
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