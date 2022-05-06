Scriptname autoBodyClearMorphs extends ActiveMagicEffect 

Event OnEffectStart(Actor akActor, Actor akCaster) 
    debug.trace("Clear Morph spell cast!")
    autoBodyUtils.ClearAllMorphs(akActor)
endEvent