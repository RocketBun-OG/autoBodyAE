#include <morphman.h>

namespace Bodygen {

// for bEnableClothingRefit
std::vector<Presets::slider> clothingsliders;
Presets::bodypreset clothingUnprocessed{clothingsliders, "Clothing"};
Presets::completedbody clothingProcessed;

Morphman *Morphman::GetInstance() {
  static Morphman instance;
  return std::addressof(instance);
};

void Morphman::initClothingSliders() {
  // booba
  clothingsliders.push_back({0.0f, 0.0f, "BreastSideShape"});
  clothingsliders.push_back({0.0f, 0.0f, "BreastUnderDepth"});
  clothingsliders.push_back({1.0f, 1.0f, "BreastCleavage"});
  clothingsliders.push_back({-0.1f, -0.05f, "BreastGravity2"});
  clothingsliders.push_back({-0.2f, -0.35f, "BreastTopSlope"});
  clothingsliders.push_back({0.3f, 0.35f, "BreastsTogether"});
  clothingsliders.push_back({-0.05f, -0.05f, "Breasts"});
  clothingsliders.push_back({0.15f, 0.15f, "BreastHeigh"});

  // butt
  clothingsliders.push_back({0.0f, 0.0f, "ButtDimples"});
  clothingsliders.push_back({0.0f, 0.0f, "ButtUnderFold"});
  clothingsliders.push_back({-0.05f, -0.05f, "AppleCheeks"});
  clothingsliders.push_back({-0.05f, -0.05f, "Butt"});

  // torso
  clothingsliders.push_back({0.0f, 0.0f, "Clavicle_v2"});
  clothingsliders.push_back({1.0f, 1.0f, "NavelEven"});
  clothingsliders.push_back({0.0f, 0.0f, "HipCarved"});

  // nipples
  clothingsliders.push_back({0.0f, 0.0f, "NippleDip"});
  clothingsliders.push_back({0.0f, 0.0f, "NippleTip"});
  clothingsliders.push_back({0.0f, 0.0f, "NipplePuffy_v2"});
  clothingsliders.push_back({-0.3f, -0.3f, "AreolaSize"});
  clothingsliders.push_back({1.0f, 1.0f, "NipBGone"});
  clothingsliders.push_back({-0.75, -0.75, "NippleManga"});
  clothingsliders.push_back({0.05f, 0.08f, "NippleDistance"});
  clothingsliders.push_back({0.0f, -0.1f, "NippleDown"});
  clothingsliders.push_back({-0.25f, -0.25f, "NipplePerkManga"});
  clothingsliders.push_back({0.0f, 0.0f, "NipplePerkiness"});

  clothingInit = true;
}

Presets::completedbody Morphman::FinishClothing(RE::Actor *a_actor) {
  auto weight = GetWeight(a_actor);
  auto clothingmods = InterpolateAllValues(clothingUnprocessed, weight);
  return clothingmods;
}

bool Morphman::GetMorphInterface(SKEE::IBodyMorphInterface *a_morphInterface) {
  return a_morphInterface->GetVersion() ? morphInterface = a_morphInterface
                                        : false;
}

// checks to see if an actor has been generated already.
bool Morphman::IsGenned(RE::Actor *a_actor) {
  return morphInterface->GetMorph(a_actor, "nobody_processed", "NoBody") ==
         1.0f;
}

// sticks a preset onto an NPC. This is the core function of the plugin, pretty
// much.
void Morphman::ApplyPreset(RE::Actor *a_actor,
                           std::vector<Presets::bodypreset> list) {

  // select a random preset from the stack
  auto preset = Presets::FindRandomPreset(list);

  // get the weight of the actor
  auto actorWeight = GetWeight(a_actor);
  // apply their weight to the preset + the offset defined in the INI
  Presets::completedbody readybody =
      InterpolateAllValues(preset, actorWeight, biasamount);

  // finally, clear off any sliders we may have put on them.
  morphInterface->ClearBodyMorphKeys(a_actor, "NoBody");
  // prep complete

  // apply the sliders to the NPC
  ApplySliderSet(a_actor, readybody, "NoBody");

  // mark the actor as generated, so we don't fuck up and generate them again
  morphInterface->SetMorph(a_actor, "nobody_processed", "NoBody", 1.0f);
  logger::info("Preset [{}] was applied to {}", readybody.presetname,
               a_actor->GetName());
  // store the actor's info just in case we need it later. Does not store it if
  // we already have done so. completedcharacter finished{readybody, a_actor,
  // a_actor->GetFormID(), a_actor->GetName(), actorWeight};
  // actorList.push_back(finished);

  return;
}

// apply a flattened slider (i.e. a single value and a name, i.e. what the morph
// interface takes in), plus a morph key
void Morphman::ApplySlider(RE::Actor *a_actor, Presets::flattenedslider slider,
                           const char *key) {
  // logger::trace("Morph {} with value {} is being applied to actor {}",
  // slider.name, slider.value, a_actor->GetName());
  morphInterface->SetMorph(a_actor, slider.name.c_str(), key, slider.value);
}

// apply an entire sliderset to a person.
void Morphman::ApplySliderSet(RE::Actor *a_actor, Presets::completedbody body,
                              const char *key) {

  // logger::trace("Applying sliderset to actor... {}", body.nodelist.size());
  for (Presets::flattenedslider node : body.nodelist) {
    // logger::trace("Applying slider {} to the actor", node.name);
    ApplySlider(a_actor, node, key);
  }

  morphInterface->ApplyBodyMorphs(a_actor, true);
}

// apply or remove clothing sliders from an actor.
void Morphman::ProcessClothing(RE::Actor *a_actor, bool unequip = false) {
  auto modifiers = FinishClothing(a_actor);
  if (unequip) {
    logger::trace("Removing clothing morphs from actor {}!",
                  a_actor->GetName());
    morphInterface->ClearBodyMorphKeys(a_actor, "NoBodyClothes");
  } else {
    logger::trace("Applying clothing morphs to actor {}!", a_actor->GetName());
    ApplySliderSet(a_actor, modifiers, "NoBodyClothes");
  }
}

// clears all changes we have made to an actor
void Morphman::FlushActor(RE::Actor *a_actor) {
  // clear the morphs.
  morphInterface->ClearBodyMorphKeys(a_actor, "NoBody");

  morphInterface->ApplyBodyMorphs(a_actor, false);
  morphInterface->UpdateModelWeight(a_actor, true);
}

// how fat are they? lul
float Morphman::GetWeight(RE::Actor *a_actor) {
  logger::trace("getting actor weight.");
  return a_actor->GetActorBase()->GetWeight() / 100.0f;
}
}; // namespace Bodygen