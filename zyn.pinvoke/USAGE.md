# zyn.pinvoke — C# gebruikshandleiding

`zyn.pinvoke.dll` is een C-compatible wrapper rond de ZynAddSubFX synthesizer. De DLL exporteert functies die via P/Invoke vanuit C# aanroepbaar zijn. De centrale abstractie is de **Mixer**: een pointer (`nint`) die de volledige synthesizer-staat beheert.

---

## P/Invoke declaraties

```csharp
using System.Runtime.InteropServices;
using System.Text;

internal static class Zyn
{
    const string Dll = "zyn.pinvoke.dll";
    const CallingConvention CC = CallingConvention.Cdecl;

    [DllImport(Dll, CallingConvention = CC)] public static extern nint CreateMixer();
    [DllImport(Dll, CallingConvention = CC)] public static extern void DestroyMixer(nint mixer);

    [DllImport(Dll, CallingConvention = CC)] public static extern void EnableTrack(nint mixer, byte track);
    [DllImport(Dll, CallingConvention = CC)] public static extern void DisableTrack(nint mixer, byte track);
    [DllImport(Dll, CallingConvention = CC)] public static extern bool IsTrackEnabled(nint mixer, byte track);
    [DllImport(Dll, CallingConvention = CC)] public static extern int  GetTrackName(nint mixer, byte track, StringBuilder buf, int len);

    [DllImport(Dll, CallingConvention = CC)] public static extern void AudioOut(nint mixer, float[] outL, float[] outR);

    [DllImport(Dll, CallingConvention = CC)] public static extern void NoteOn (nint mixer, byte track, byte note, byte velocity);
    [DllImport(Dll, CallingConvention = CC)] public static extern void NoteOff(nint mixer, byte track, byte note);

    [DllImport(Dll, CallingConvention = CC)]
    public static extern void LoadPresets(nint mixer, byte track,
        [MarshalAs(UnmanagedType.LPStr)] string xmldata);

    [DllImport(Dll, CallingConvention = CC)]
    public static extern void LoadPresetsFromFile(nint mixer, byte track,
        [MarshalAs(UnmanagedType.LPStr)] string filePath);

    [DllImport(Dll, CallingConvention = CC)]
    public static extern void LoadPresetFromBank(nint mixer, byte track,
        [MarshalAs(UnmanagedType.LPStr)] string bankName, int slot);

    // Parameters
    [DllImport(Dll, CallingConvention = CC)]
    public static extern byte  GetPar    (nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void  SetPar    (nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id, byte value);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern bool  GetParBool(nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void  SetParBool(nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id, bool value);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern float GetParReal(nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void  SetParReal(nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id, float value);

    // Effects
    [DllImport(Dll, CallingConvention = CC)]
    public static extern int  GetEffect        (nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void ChangeEffect     (nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id, int effectType);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void ChangeEffectPreset(nint mixer, byte track, [MarshalAs(UnmanagedType.LPStr)] string id, int preset);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern float GetFrequencyResponse(nint mixer, byte track,
        [MarshalAs(UnmanagedType.LPStr)] string id, float frequency);

    // Samples (SampleSynth / drumkit)
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void SetSample(nint mixer, byte track, byte instrumentIndex,
        [MarshalAs(UnmanagedType.LPStr)] string samplePath);
    [DllImport(Dll, CallingConvention = CC)]
    public static extern void SetSampleData(nint mixer, byte track, byte instrumentIndex,
        int channels, int samplesPerChannel,
        [MarshalAs(UnmanagedType.LPStr)] string sampleData);

    // Banks
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetBankCount();
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetBankIndex([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetBankName(int bankIndex, StringBuilder buf, int len);
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetBankPath(int bankIndex, StringBuilder buf, int len);
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetInstrumentCount(int bankIndex);
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetInstrumentName(int bankIndex, int slot, StringBuilder buf, int len);
    [DllImport(Dll, CallingConvention = CC)] public static extern int GetInstrumentPath(int bankIndex, int slot, StringBuilder buf, int len);
}
```

> **Let op:** De C++ kant gebruikt `unsigned char` voor `trackIndex` en parameterwaardes. In C# declareer je dit als `byte` (niet `char`).

---

## Basisgebruik

```csharp
// Aanmaken
nint mixer = Zyn.CreateMixer();

// Track activeren en preset laden
Zyn.EnableTrack(mixer, 0);
Zyn.LoadPresetFromBank(mixer, 0, "GeneralUser", 42);

// Audio genereren (buffergrootte is vaste 256 samples)
var outL = new float[256];
var outR = new float[256];
Zyn.AudioOut(mixer, outL, outR);

// Noten
Zyn.NoteOn(mixer, 0, 60, 100);   // track 0, C4, velocity 100
Zyn.NoteOff(mixer, 0, 60);

// Opruimen
Zyn.DestroyMixer(mixer);
```

---

## Parameter-ID systeem

Alle synthesizer-parameters worden gelezen en geschreven via een **string-ID**. De ID beschrijft het pad door de objecthiërarchie, gescheiden door punten. Array-indexen staan tussen rechte haken.

### Typen

| Functie         | C++ type        | Gebruik voor                         |
|-----------------|-----------------|--------------------------------------|
| `GetPar`/`SetPar`     | `unsigned char` (0–255) | Vrijwel alle parameters    |
| `GetParBool`/`SetParBool` | `bool`      | Vlaggen (enabled, bypass, ...)       |
| `GetParReal`/`SetParReal` | `float`     | Alleen LFO-frequentie (`Pfreq`)      |

### ID-opbouw

```
{scope}.{pad}
```

**Scope** is altijd `Track` voor track-gebonden parameters, of leeg voor mixer-niveau.

```
Pvolume                                          ← mixer master volume
Pkeyshift                                        ← mixer master keyshift
SystemEffects[0].Parameters[2]                   ← system effect 0, parameter 2
InsertEffects[3].Parameters[0]                   ← insert effect 3, parameter 0
Track.Pvolume                                    ← track volume (huidig track)
Track.Instruments[0].AddSynth.PVolume            ← ADnote instrument 0 volume
```

---

## Volledige parameter-referentie

### Mixer (geen trackIndex nodig, geef 0)

| ID | Type | Beschrijving |
|----|------|--------------|
| `Pvolume` | byte | Master volume (0–127) |
| `Pkeyshift` | byte | Master key shift |
| `SystemEffects[n].Parameters[p]` | byte | System effect n (0–3), parameter p |
| `InsertEffects[n].Parameters[p]` | byte | Insert effect n (0–7), parameter p |

### Track

| ID | Type | Beschrijving |
|----|------|--------------|
| `Track.Pvolume` | byte | Track volume |
| `Track.Ppanning` | byte | Track panning |
| `Track.Pkeylimit` | byte | Key limit |
| `Track.Pvelsns` | byte | Velocity gevoeligheid |
| `Track.Pveloffs` | byte | Velocity offset |
| `Track.Pminkey` | byte | Minimale noot |
| `Track.Pmaxkey` | byte | Maximale noot |
| `Track.Pkeyshift` | byte | Key shift |
| `Track.Prcvchn` | byte | MIDI receive channel |
| `Track.Pkitmode` | byte | Kit mode |
| `Track.Pfxsend1`–`Pfxsend4` | byte | Send-niveau naar system effect 1–4 |
| `Track.Effects[n].Parameters[p]` | byte | Track insert effect n (0–2), parameter p |

### Instrument

Prefix: `Track.Instruments[i].` (i = 0–15)

| ID | Type | Beschrijving |
|----|------|--------------|
| `Padenabled` | byte | ADnote aan/uit |
| `Psubenabled` | byte | SUBnote aan/uit |
| `Ppadenabled` | byte | PADnote aan/uit |
| `Psmplenabled` | byte | SampleSynth aan/uit |
| `Psendtoparteffect` | byte | Routing naar track effect |
| `Pminkey` / `Pmaxkey` | byte | Key range |

---

### Gedeelde parameters (AbstractNote)

Van toepassing op `AddSynth`, `SubSynth`, `PadSynth` en `SampleSynth`.  
Prefix: `Track.Instruments[i].{Synth}.`

| ID | Type | Beschrijving |
|----|------|--------------|
| `PVolume` | byte | Volume |
| `PPanning` | byte | Panning |
| `PAmpVelocityScaleFunction` | byte | Amplitude velocity schaal |
| `Pfixedfreq` | byte | Vaste frequentie aan/uit |
| `PfixedfreqET` | byte | Vaste frequentie gelijkzwevend temperament |
| `PDetune` | byte | Detune (fijn) |
| `PCoarseDetune` | byte | Detune (grof) |
| `PDetuneType` | byte | Detune type |
| `PFreqEnvelopeEnabled` | byte | Frequentie envelope aan/uit |
| `FrequencyEnvelope.{par}` | byte | Zie [Envelope parameters](#envelope-parameters) |
| `PFreqLfoEnabled` | byte | Frequentie LFO aan/uit |
| `FrequencyLfo.{par}` | byte/float | Zie [LFO parameters](#lfo-parameters) |
| `PBandwidth` | byte | Bandbreedte |
| `PAmpEnvelopeEnabled` | byte | Amplitude envelope aan/uit |
| `AmpEnvelope.{par}` | byte | Zie [Envelope parameters](#envelope-parameters) |
| `GlobalFilter.{par}` | byte | Zie [Filter parameters](#filter-parameters) |
| `PFilterVelocityScale` | byte | Filter velocity schaal |
| `PFilterVelocityScaleFunction` | byte | Filter velocity schaalf. |
| `FilterEnvelope.{par}` | byte | Zie [Envelope parameters](#envelope-parameters) |

#### Envelope parameters

Suffix na `{EnvelopeName}.`:

| Naam | Beschrijving |
|------|--------------|
| `PAttackTime` | Attack tijd |
| `PAttackValue` | Attack waarde |
| `PDecayTime` | Decay tijd |
| `PDecayValue` | Decay waarde |
| `PSustainValue` | Sustain waarde |
| `PReleaseTime` | Release tijd |
| `PReleaseValue` | Release waarde |
| `Pforcedrelease` | Forceer release |
| `Plinearenvelope` | Lineaire envelope |
| `PStretch` | Stretch |

#### LFO parameters

Suffix na `{LfoName}.`:

| Naam | Type | Beschrijving |
|------|------|--------------|
| `Pfreq` | **float** | Frequentie |
| `Pintensity` | byte | Intensiteit |
| `Pstartphase` | byte | Startfase |
| `PLFOtype` | byte | LFO type (0=sinus, 1=driehoek, ...) |
| `Prandomness` | byte | Willekeur |
| `Pfreqrand` | byte | Frequentie willekeur |
| `Pdelay` | byte | Vertraging |
| `Pcontinous` | byte | Continu (niet resetten bij noot) |
| `Pstretch` | byte | Stretch |

#### Filter parameters

Suffix na `{FilterName}.`:

| Naam | Beschrijving |
|------|--------------|
| `Pcategory` | Categorie (0=analog, 1=formant, 2=SVF) |
| `Ptype` | Filter type |
| `Pfreq` | Cutoff frequentie |
| `Pq` | Resonantie (Q) |
| `Pstages` | Aantal stadia |
| `Pfreqtrack` | Frequentie tracking |
| `Pgain` | Gain |
| `Pnumformants` | Aantal formanten (formant modus) |
| `Pformantslowness` | Formant traagheid |
| `Pvowelclearness` | Klaarheid klinker |
| `Pcenterfreq` | Centrumfrequentie |
| `Poctavesfreq` | Octaven frequentiebereik |

---

### ADnote (AddSynth)

Prefix: `Track.Instruments[i].AddSynth.`

| ID | Beschrijving |
|----|--------------|
| `AmpLfo.{par}` | Globale amplitude LFO |
| `FilterLfo.{par}` | Globale filter LFO |
| `PPunchStrength` | Punch sterkte |
| `PPunchTime` | Punch tijd |
| `PPunchStretch` | Punch stretch |
| `PPunchVelocitySensing` | Punch velocity |
| `Hrandgrouping` | Harmonische willekeur groepering |
| `Reson.{par}` | Zie [Resonance parameters](#resonance-parameters) |
| `Voices[v].{par}` | Voice v (0–7), zie [ADnote Voice](#adnote-voice) |

#### ADnote Voice

Prefix: `Track.Instruments[i].AddSynth.Voices[v].` (v = 0–7)

Erft alle [gedeelde parameters](#gedeelde-parameters-abstractnote), plus:

| ID | Beschrijving |
|----|--------------|
| `Enabled` | Voice aan/uit |
| `PStereo` | Stereo |
| `Unison_size` | Unison stemmen |
| `Unison_frequency_spread` | Unison frequentiespreiding |
| `Unison_phase_randomness` | Unison fasewillekeur |
| `Unison_stereo_spread` | Unison stereobreedte |
| `Unison_vibratto` | Unison vibrato diepte |
| `Unison_vibratto_speed` | Unison vibrato snelheid |
| `Unison_invert_phase` | Unison fase inversie |
| `Type` | Voice type |
| `PDelay` | Vertraging |
| `Presonance` | Resonantie aan/uit |
| `Pextoscil` | Externe oscillator index (-1 = intern) |
| `PextFMoscil` | Externe FM-oscillator index |
| `Poscilphase` | Oscillator startfase |
| `PFMoscilphase` | FM-oscillator startfase |
| `Pfilterbypass` | Filter bypass |
| `PVolumeminus` | Volume inversie |
| `PAmpLfoEnabled` | Amplitude LFO aan/uit |
| `AmpLfo.{par}` | Amplitude LFO |
| `PFilterEnabled` | Filter aan/uit |
| `VoiceFilter.{par}` | Voice filter |
| `PFilterEnvelopeEnabled` | Filter envelope aan/uit |
| `PFilterLfoEnabled` | Filter LFO aan/uit |
| `FilterLfo.{par}` | Filter LFO |
| `PFMEnabled` | FM modulatie aan/uit |
| `PFMVoice` | FM bronvoice (-1 = intern) |
| `PFMVolume` | FM volume |
| `PFMVolumeDamp` | FM volume demping |
| `PFMVelocityScaleFunction` | FM velocity schaal |
| `PFMDetune` | FM detune |
| `PFMCoarseDetune` | FM grove detune |
| `PFMDetuneType` | FM detune type |
| `PFMFreqEnvelopeEnabled` | FM frequentie envelope aan/uit |
| `FMFreqEnvelope.{par}` | FM frequentie envelope |
| `PFMAmpEnvelopeEnabled` | FM amplitude envelope aan/uit |
| `FMAmpEnvelope.{par}` | FM amplitude envelope |
| `OscilSmp.{par}` | Oscillator — zie [OscilGen parameters](#oscilgen-parameters) |
| `FMSmp.{par}` | FM-modulator oscillator — zie [OscilGen parameters](#oscilgen-parameters) |

---

### SUBnote (SubSynth)

Prefix: `Track.Instruments[i].SubSynth.`

Erft alle [gedeelde parameters](#gedeelde-parameters-abstractnote), plus:

| ID | Beschrijving |
|----|--------------|
| `PBandWidthEnvelopeEnabled` | Bandbreedte envelope aan/uit |
| `BandWidthEnvelope.{par}` | Bandbreedte envelope |
| `PGlobalFilterEnabled` | Globaal filter aan/uit |
| `POvertoneSpread.type` | Boventoon spreiding type |
| `POvertoneSpread.par1`–`par3` | Boventoon spreiding parameters |
| `Pnumstages` | Aantal filterstadia |
| `Phmagtype` | Magnitude schaal type |
| `Pbwscale` | Bandbreedte schaling |
| `Pstart` | Starttype |
| `POvertoneFreqMult[n]` | Boventoon frequentiemultiplier n (0–63) |
| `Phmag[n]` | Harmonische magnitude n (0–63) |
| `Phrelbw[n]` | Relatieve bandbreedte harmoniek n (0–63) |

---

### PADnote (PadSynth)

Prefix: `Track.Instruments[i].PadSynth.`

Erft alle [gedeelde parameters](#gedeelde-parameters-abstractnote), plus:

| ID | Beschrijving |
|----|--------------|
| `Pbwscale` | Bandbreedte schaling |
| `AmpLfo.{par}` | Amplitude LFO |
| `PPunchStrength` | Punch sterkte |
| `PPunchTime` | Punch tijd |
| `PPunchStretch` | Punch stretch |
| `PPunchVelocitySensing` | Punch velocity |
| `FilterLfo.{par}` | Filter LFO |
| `oscilgen.{par}` | Oscillator — zie [OscilGen parameters](#oscilgen-parameters) |
| `resonance.{par}` | Resonantie — zie [Resonance parameters](#resonance-parameters) |

> **Let op:** PADnote gebruikt lowercase `oscilgen` en `resonance` als ID-segment (in tegenstelling tot ADnote dat `Reson` gebruikt).

---

### OscilGen parameters

Gebruikt als suffix bij:
- `Track.Instruments[i].AddSynth.Voices[v].OscilSmp.{par}`
- `Track.Instruments[i].AddSynth.Voices[v].FMSmp.{par}`
- `Track.Instruments[i].PadSynth.oscilgen.{par}`

| ID | Beschrijving |
|----|--------------|
| `Phmag[n]` | Harmonische magnitude n (0–127, waarde 0–127 waarbij 64 = nul) |
| `Phphase[n]` | Harmonische fase n (0–127) |
| `Phmagtype` | Magnitudetschaal: 0=lineair, 1=dB40, 2=dB60, 3=dB80, 4=dB100 |
| `Pcurrentbasefunc` | Basisgolfvorm (0=sinus, 1=driehoek, 2=puls, ...) |
| `Pbasefuncpar` | Basisgolfvorm parameter |
| `Pbasefuncmodulation` | Modulatie op basisgolfvorm |
| `Pbasefuncmodulationpar1`–`par3` | Basisgolfvorm modulatieparameters |
| `Prand` | Willekeur: 64=geen, 0–63=blok, 65–127=per harmoniek |
| `Pwaveshaping` | Waveshaping intensiteit |
| `Pwaveshapingfunction` | Waveshaping functie type |
| `Pfiltertype` | Oscillator filter type |
| `Pfilterpar1` / `Pfilterpar2` | Oscillator filter parameters |
| `Pfilterbeforews` | Filter vóór waveshaping toepassen |
| `Psatype` | Spectrum aanpassing type |
| `Psapar` | Spectrum aanpassing parameter |
| `Pamprandpower` | Amplitude willekeur sterkte |
| `Pamprandtype` | Amplitude willekeur type |
| `Pharmonicshiftfirst` | Harmonische verschuiving vóór waveshaping |
| `Padaptiveharmonics` | Adaptieve harmonischen modus |
| `Padaptiveharmonicsbasefreq` | Adaptieve harmonischen basisfrequentie |
| `Padaptiveharmonicspower` | Adaptieve harmonischen sterkte |
| `Padaptiveharmonicspar` | Adaptieve harmonischen parameter |
| `Pmodulation` | Oscillator modulatie type |
| `Pmodulationpar1`–`par3` | Oscillator modulatieparameters |

**Harmonischen instellen** — voorbeeld alle 128 harmonischen uitlezen:
```csharp
for (int h = 0; h < 128; h++)
{
    byte mag = Zyn.GetPar(mixer, 0,
        $"Track.Instruments[0].AddSynth.Voices[0].OscilSmp.Phmag[{h}]");
}
```

---

### Resonance parameters

Gebruikt als suffix bij:
- `Track.Instruments[i].AddSynth.Reson.{par}`
- `Track.Instruments[i].PadSynth.resonance.{par}`

| ID | Beschrijving |
|----|--------------|
| `Penabled` | Resonantie aan/uit |
| `PmaxdB` | Maximale versterking in dB |
| `Pcenterfreq` | Centrumfrequentie |
| `Poctavesfreq` | Frequentiebereik in octaven |
| `Pprotectthefundamental` | Grondtoon niet dempen |
| `Prespoints[n]` | Resonantiecurve punt n (0–255, waarde 0–255) |

**Resonantiecurve instellen** — voorbeeld een piek op punt 128:
```csharp
string resonBase = "Track.Instruments[0].AddSynth.Reson";
Zyn.SetPar(mixer, 0, $"{resonBase}.Penabled", 1);
Zyn.SetPar(mixer, 0, $"{resonBase}.Prespoints[128]", 200);
```

---

## Effects

Effect-ID's voor `GetEffect` / `ChangeEffect` / `ChangeEffectPreset` / `GetFrequencyResponse`:

| ID | Beschrijving |
|----|--------------|
| `SystemEffects[n]` | System effect n (0–3) |
| `InsertEffects[n]` | Insert effect n (0–7) |
| `Track.Effects[n]` | Track insert effect n (0–2) |

Effect types (int voor `ChangeEffect`):

| Waarde | Effect |
|--------|--------|
| 0 | Geen |
| 1 | Reverb |
| 2 | Echo |
| 3 | Chorus |
| 4 | Phaser |
| 5 | AlienWah |
| 6 | Distortion |
| 7 | EQ |
| 8 | DynFilter |

Effect-parameters worden gelezen/geschreven via `GetPar`/`SetPar` met het pad `{EffectId}.Parameters[p]`.

---

## Banks en presets

```csharp
// Alle beschikbare banken ophalen
int count = Zyn.GetBankCount();
var buf = new StringBuilder(256);
for (int i = 0; i < count; i++)
{
    Zyn.GetBankName(i, buf, 256);
    Console.WriteLine($"Bank {i}: {buf}");
}

// Index van een bank op naam
int idx = Zyn.GetBankIndex("GeneralUser");

// Instrumenten in een bank
int instruments = Zyn.GetInstrumentCount(idx);
Zyn.GetInstrumentName(idx, 0, buf, 256);

// Preset laden vanuit bank
Zyn.LoadPresetFromBank(mixer, 0, "GeneralUser", 42);

// Preset laden vanuit XML-bestand
Zyn.LoadPresetsFromFile(mixer, 0, @"C:\presets\mijn_preset.xmz");

// Preset laden vanuit XML-string
Zyn.LoadPresets(mixer, 0, xmlString);
```

---

## Constanten

| Naam | Waarde | Beschrijving |
|------|--------|--------------|
| Tracks | 16 | Maximaal aantal tracks per mixer |
| Instruments per track | 16 | Maximaal aantal instrumenten per track |
| Voices per ADnote | 8 | Aantal voices in AddSynth |
| ADnote harmonischen | 128 | Aantal harmonischen per OscilGen |
| Resonantiepunten | 256 | Aantal punten in resonantiecurve |
| SUBnote harmonischen | 64 | Aantal harmonischen in SubSynth |
| System effects | 4 | |
| Insert effects | 8 | |
| Track effects | 3 | |
| Audio buffergrootte | 256 samples | Vaste grootte voor `AudioOut` |
| Sample rate | 44100 Hz | |

---

## Logbestand

De DLL schrijft diagnose-uitvoer naar:
```
C:\Users\{gebruiker}\AppData\Local\Temp\mixer-log.txt
```
Dit bestand wordt aangemaakt bij `CreateMixer()` en gesloten bij `DestroyMixer()`.
