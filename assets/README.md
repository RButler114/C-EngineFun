# Game Assets

Audio assets for the game engine.

## Audio Files

### Sound Effects (`sounds/`)
- `jump.wav` - Player jump sound
- `collision.wav` - Collision/impact sound
- `menu_select.wav` - Menu navigation sound

### Music (`music/`)
- `background.wav` - Background music sample

## Usage

```cpp
// Load and play sounds
audioManager.LoadSound("jump", "assets/sounds/jump.wav", SoundType::SOUND_EFFECT);
audioManager.PlaySound("jump", 0.8f);

// Load and play music
audioManager.LoadMusic("background", "assets/music/background.wav");
audioManager.PlayMusic("background", 0.5f, -1);  // 50% volume, loop
```

## Supported Formats
- **WAV** (recommended for sound effects)
- **OGG** (recommended for music)
- **MP3**, **FLAC**

## Adding Custom Audio
Replace the sample files with your own audio assets. Recommended settings:
- Sample Rate: 44100 Hz
- Channels: Stereo
- Format: WAV for effects, OGG for music
