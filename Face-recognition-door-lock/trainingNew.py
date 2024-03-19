import cv2
import numpy as np
import os
import RPi.GPIO as GPIO
import time
import requests
from picamera import PiCamera
from datetime import datetime

# Initialize Telegram bot
BOT_TOKEN = '5898372055:AAHVES0MXlnR-IgQBn5Se9PXV_NeiuRMW3s'
CHAT_ID = 'YOUR_CHAT_ID_HERE'

# Initialize GPIO
relay = 23
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(relay, GPIO.OUT)
GPIO.output(relay ,1)

# Load face recognition model
recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read('trainer.yml')

# Load face detection model
cascadePath = "haarcascade_frontalface_default.xml"
faceCascade = cv2.CascadeClassifier(cascadePath)

# Define font for text overlay
font = cv2.FONT_HERSHEY_SIMPLEX

# Define names related to ids
names = ['None', 'Manisha', 'Nikat', 'Ayman'] 

# Initialize camera
cam = cv2.VideoCapture(-1)
cam.set(3, 640) # Set video width
cam.set(4, 480) # Set video height

# Define min window size to be recognized as a face
minW = 0.3*cam.get(3)
minH = 0.3*cam.get(4)

# Initialize PiCamera
camera = PiCamera()
camera.rotation = 180

while True:
    ret, img = cam.read()
    img = cv2.flip(img, -1) # Flip vertically
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    faces = faceCascade.detectMultiScale( 
        gray,
        scaleFactor=1.2,
        minNeighbors=5,
        minSize=(int(minW), int(minH)),
       )
    for(x, y, w, h) in faces:
        cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
        id, confidence = recognizer.predict(gray[y:y+h, x:x+w])
        # Check if confidence is less than 55 ==> "0" is a perfect match
         if (confidence <55):
            id = names[id]
            #confidence = "  {0}%".format(round(100 - confidence))
            GPIO.output(relay, 0)
            print("Opening Lock")
            # Capture the image and save it to a temporary file
            _, frame = cam.read()
            temp_file = 'temp.jpg'
            cv2.imwrite(temp_file, frame)
            # Send the message with the image to the Telegram channel/group
            url = f'https://api.telegram.org/bot{BOT_TOKEN}/sendPhoto'
            files = {'photo': open(temp_file, 'rb')}
            data = {'chat_id': CHAT_ID, 'caption': f'{id} unlocked the lock'}
            response = requests.post(url, files=files, data=data)
            print(response.json())
            time.sleep(1)
            GPIO.output(relay, 1)
            print("Closing Lock")
            os.remove(temp_file) # Remove the temporary file
            else:
            name = "unknown"
            confidence = "  {0}%".format(round(100 - confidence))
            GPIO.output(relay, 1)
            #confidence = " {0}%".format(round(100 - confidence))
        cv2.putText(img, str(name), (x+5, y-5), font, 1, (255, 255, 255), 2)
        cv2.putText(img, str(confidence), (x+5, y+h-5), font, 1, (255, 255, 0), 1) 
        #cv2.putText(img, str(confidence), (x+5,y+h-5), font, 1, (255,255,0), 1)
        cv2.imshow('camera', img)
    k = cv2.waitKey(10) & 0xff # Press 'ESC' for exiting
    if k == 47:
        break
# Do a bit of cleanup
print("\n [INFO] Exiting Program and cleanup stuff")
cam.release()
cv2.destroyAllWindows()






         
    