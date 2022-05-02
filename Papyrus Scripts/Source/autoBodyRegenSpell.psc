Scriptname autoBodyRegenSpell extends ActiveMagicEffect 

Event OnEffectStart(Actor akActor, Actor akCaster) 
    debug.trace("Regen spell cast!")
    autoBodyUtils.RegenActor(akActor)
endEvent


