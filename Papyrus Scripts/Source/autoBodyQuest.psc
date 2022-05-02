ScriptName autoBodyQuest extends Quest

Actor PlayerRef

Actor Property TargetDiscriminate
    Actor Function Get()
        Actor out = Game.GetCurrentCrosshairRef() as Actor

        if !out
            out = PlayerRef
        EndIf

        return out

    EndFunction
endProperty

int Property PresetKey
    int Function Get()
        return 39
    endFunction
endProperty


Event OnInit()
    PlayerRef = Game.GetPlayer()
    String[] FemalePool = autoBodyUtils.GetMasterPresetPool(true)
    String[] MalePool = autoBodyUtils.GetMasterPresetPool(false)

    int femaleSize = FemalePool.Length
    int maleSize = MalePool.Length

    debug.Notification("autoBody Online! List Size: [F: " + femaleSize + "] [M: " + maleSize + "]")
    OnLoad()

EndEvent

Function OnLoad()
    RegisterForKey(PresetKey)
EndFunction

Event OnGameLoad()
    OnLoad()
EndEvent

Event OnKeyDown(int KeyPress)
    if KeyPress == PresetKey
        ShowMenu(TargetDiscriminate)
    endif
endEvent

Function ShowMenu(Actor akActor)
    Debug.Notification("Editing " + akActor.GetDisplayName())
    UIListMenu listMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
    listMenu.ResetMenu()
    
    
    String[] title = new String[1]
    title[0] = "- Bolt-on Workshop -"
    
    ActorBase akBase = akActor.GetActorBase()
    String[] presets
    if (akBase.GetSex() == 0)
         presets = autoBodyUtils.GetMasterPresetPool(false)
    elseif (akBase.GetSex() == 1)
         presets = autoBodyUtils.GetMasterPresetPool(true)
    endif

    int len = presets.Length
    


    int pagesNeeded
    if len > 125
        pagesNeeded = (1 / 125) + 1

        while i <= pagesNeeded
            listMenu.AddEntryItem("Presets: Page " + (i + 1))
            i += 1
        EndWhile

        listMenu.OpenMenu(akActor)
        int num = listMenu.GetResultInt()
        if num < 0
            return
        endif 

        int start = num * 125
        int end 
        if num == (pagesNeeded - 1)
            end = presets.Length - 1
        Else
            end = start + 124
        EndIf

        listMenu.ResetMenu()
        presets = PapyrusUtil.SliceStringArray(presets, start, end)
    endif

    presets= PapyrusUtil.MergeStringArray(title, presets)

    int i = 0
    int max = presets.Length
    While (i < max)
        listMenu.AddEntryItem(presets[i])
        i += 1
    endWhile

    listMenu.OpenMenu(akActor)
    string result = listMenu.GetResultString()

    int num = listMenu.GetResultInt()
    if !(num < 1) 
        autoBodyUtils.ApplyPresetByName(akActor, result)
    EndIf
EndFunction