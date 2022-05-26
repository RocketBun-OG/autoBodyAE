Scriptname autoBodyUtils

Function RegenActor(Actor a_actor) Global Native
 
Function SetINIKey(String sectionname, String keyname, string value) Global Native

Function ApplyPresetByName(Actor a_actor, String presetname) Global Native

String[] Function GetActorPresetPool(Actor a_actor) Global Native

String[] Function GetMasterPresetPool(bool female) Global Native

String[] Function GetBackupMasterPool(bool female) Global Native

bool Function GetActorGenerated(Actor a_actor) Global Native

Function ClearAllMorphs(Actor a_acor) Global Native