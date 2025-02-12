from qiskit import QuantumCircuit, transpile
from qiskit_ibm_runtime import QiskitRuntimeService, Sampler
import serial
import numpy as np
import time

arduino = serial.Serial(port='COM3', baudrate=9600)

circuit = QuantumCircuit(1, 1)
circuit.h(0)
circuit.measure(0, 0)

service = QiskitRuntimeService(channel='ibm_quantum', token='ce3c1b28ec49acf152315e47ab8bbe0745e6a12f5fccc00a766a2fb193b745a7b5518cdc3f38e2a92c7a716b43f330472b02c38ae2998dfc2ce0a5f7bcaf94e2')
backend = service.least_busy(operational=True, simulator=False)
circuit = transpile(circuit, backend)

sampler = Sampler(backend)
job = sampler.run([circuit])

while job.status() in ['QUEUED', 'RUNNING']:
    print(f"Job status: {job.status()}, Waiting...")
    time.sleep(1)

result = job.result()
print(result)


bit_data = result
qb = 1 if bit_data[0] else 0

arduino.reset_input_buffer()


arduino.write(bytes([qb]))

print(f"Sent to Arduino: {qb}")

time.sleep(1)
response = arduino.readline().decode().strip()
print(f"Arduino Response: {response}")

arduino.close()
