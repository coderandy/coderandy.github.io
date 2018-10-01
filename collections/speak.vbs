Dim Message, Speak
Message=InputBox("Enter a line of text to be spoken by the computer","Speak")
Set Speak=CreateObject("sapi.spvoice")
Speak.Speak Message
