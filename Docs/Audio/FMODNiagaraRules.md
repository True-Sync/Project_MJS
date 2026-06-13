# FMOD Niagara Rules

Project_MJS uses FMODStudio for gameplay audio and FMODStudioNiagara for VFX-bound audio.

## Use Niagara FMOD modules when

- The sound is part of a Niagara effect lifecycle.
- The sound should start and stop with a particle system.
- The sound is visual-only feedback, such as aura hum, projectile trail, impact sparks, or charge particles.
- The sound does not decide gameplay state.

## Use USoundManagerSubsystem when

- The sound depends on gameplay state, hit result, combo state, boss phase, or UI state.
- The sound affects or follows BGM, pause, volume, ambience, or save settings.
- The sound needs cooldown or loop handle management.
- The sound is player action feedback such as attack swing, hit, dash, skill start/end, or footstep.

## Recommended split

- Niagara: trail loops, persistent VFX hums, particle impacts that are purely visual.
- SoundManagerSubsystem: BGM, ambience, UI, voice, combat results, attack decisions, boss phase changes.
- AnimNotify: animation-timed one-shots and loop ranges.

## Naming

- FMOD events: `event:/SFX/...`, `event:/Music/...`, `event:/Ambience/...`, `event:/UI/...`
- FMOD buses: `bus:/Master`, `bus:/Music`, `bus:/SFX`, `bus:/UI`, `bus:/Voice`, `bus:/Ambience`
- Niagara systems should not own BGM, UI, or global state audio.
