from flask import Flask, render_template, Response, request, redirect, url_for, jsonify
app = Flask(__name__)
import socket
import time
sock=socket.socket()
sonar_sock=socket.socket()
esps=socket.socket()
LEFT, RIGHT, FORWARD, BACKWARD = "left", "right", "forward", "backward"
AVAILABLE_COMMANDS = {
    'Left': LEFT,
    'Right': RIGHT,
    'Forward': FORWARD,
    'Backward': BACKWARD
    
}

@app.route('/')
def execute():
    return render_template('index.html', commands=AVAILABLE_COMMANDS)
@app.route('/espconnect')
def espconnect():
	esps.connect(('192.168.1.103', 1080))
@app.route('/espdata')
def espdata():
	espgetdata ="getespdata"
	esps.send(espgetdata.encode())
	espdata = esps.recv(1024)
	time.sleep(1)
	return jsonify(espdata)
@app.route('/sonar')
def sonar():
	sonar_command="sonar"
	sonar_sock.send(sonar_command.encode())
	sonar_data = sonar_sock.recv(1024)
	return jsonify(sonar_data)
@app.route('/connect')
def connect():
	sock.connect(('192.168.1.104', 1080))
	sonar_sock.connect(('192.168.1.104', 1080))
@app.route('/disconnect')
def disconnect():
	sock.shutdown(1)
	sock.close()
	sonar_sock.shutdown(1)
	sonar_sock.close()
@app.route('/<cmd>')
def command(cmd=None):
        move_command = cmd
	sock.send(move_command.encode())
        return render_template('index.html')
