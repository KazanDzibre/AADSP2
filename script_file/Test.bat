"model0" "speech.wav" "speech_model0_output.wav" "20000" "2000"


"model1" "speech.wav" "speech_model1_output.wav" "20000" "2000"


"model2" "speech.wav" "speech_model2_output.wav" "20000" "2000"




PCMCompare "speech_model1_output.wav" "speech_model2_output.wav" > LogFile/mo1vsMo2Speech.txt



PCMCompare "speech_model0_output.wav" "speech_model1_output.wav" > LogFile/mo0vsMo1Speech.txt

