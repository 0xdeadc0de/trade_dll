#Persistent

+LButton:: ; Start sending clicks when Left Shift + Left Mouse Button is pressed
    Loop 25 {
        Click
        Sleep 1  ; Adjust the sleep time (in milliseconds) between clicks if needed
    }
return

-:: ; Start sending NumpadSub when '-' key is pressed
    Loop 7 { 
        SendInput, {NumpadSub} 
        Sleep, 1 ;
    }
return

=:: ; Start sending NumpadAdd when '=' key is pressed
    Loop 7 { 
        SendInput, {NumpadAdd}
        Sleep, 1 ;
    }
return

a::Left
s::Down
d::Right
w::Up

