# AE Compressor 0.2 — fixed build

JUCE 8 VST3 compressor for Windows.

## Важно при качване в GitHub
В корена на repository-то трябва да виждаш директно:

- `.github`
- `src`
- `CMakeLists.txt`
- `README.md`

Не трябва да има допълнителна папка `AE-Compressor` над тях.

## Build
1. Качи **съдържанието** на тази папка в празно GitHub repository.
2. Отвори **Actions**.
3. Избери **Build AE Compressor VST3**.
4. Натисни **Run workflow**.
5. След зеления build свали artifact `AE-Compressor-vst3-windows`.
6. Копирай `AE Compressor.vst3` в `C:\Program Files\Common Files\VST3`.
7. В FL Studio: **Options → Manage plugins → Find installed plugins**.

## Контроли
Threshold, Input, Output, Ratio, Attack, Release, Make-up, Mix, Soft/Medium/Hard Knee, Auto Gain, Bass Ignore, meters and eight presets. No Lookahead.
