import socket
import dht
import machine
d = dht.DHT11(machine.Pin(4))
adc = machine.ADC(0)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("192.168.43.197", 1080))
sock.listen(3)
conn, addr = sock.accept()
button_num = 0
while 1:
	data=conn.recv(1024).decode()
	if data == 'getespdata':
		d.measure()
		sensors = [d.temperature(),d.humidity(),adc.read()]
		conn.send(sensors)
		
