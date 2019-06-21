"model0" "speech.wav" "speech_model0_output.wav"

"model0" "titanic_horn.wav" "titanic_horn_model0_output.wav"

:PCMCompare "WhiteNoise_output_model_0.wav" "WhiteNoise_output_model_1.wav" > LogFile/mo0vsMo1.txt

:PCMCompare "WhiteNoise_output_model_1.wav" "WhiteNoise_output_model_2.wav" > LogFile/mo1vsMo2.txt