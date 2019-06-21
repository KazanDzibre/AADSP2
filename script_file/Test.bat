"model0" "speech.wav" "speech_model0_output.wav"

"model0" "titanic_horn.wav" "titanic_horn_model0_output.wav"

"model1" "speech.wav" "speech_model1_output.wav"

"model1" "titanic_horn.wav" "titanic_horn_model1_output.wav"

PCMCompare "speech_model0_output.wav" "speech_model1_output.wav" > LogFile/mo0vsMo1.txt

PCMCompare "titanic_horn_model0_output.wav" "titanic_horn_model1_output.wav" > LogFile/mo1vsMo2.txt