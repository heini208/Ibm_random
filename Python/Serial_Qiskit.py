import json
import time
import glob
from qiskit import QuantumCircuit
from qiskit_ibm_runtime import QiskitRuntimeService, Sampler
from qiskit_aer import Aer
from serial import Serial


def find_arduino_port() -> str:
    ports = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
    if not ports:
        print("No Arduino found.")
        return ""
    print(ports[0])
    return ports[0]


def configure_ibm_token(token: str) -> bytes:
    QiskitRuntimeService.save_account(token, overwrite=True)
    print("IBM Quantum token configured.")
    return json.dumps({"status": "IBM token configured"}).encode('utf-8')


def generate_superposition_qubits_simulated(num_qubits: int) -> list[int]:
    circuit = QuantumCircuit(num_qubits, num_qubits)
    circuit.h(range(num_qubits))
    circuit.measure(range(num_qubits), range(num_qubits))

    simulator = Aer.get_backend('qasm_simulator')
    job = simulator.run([circuit])
    result = job.result()
    counts = result.get_counts()

    return list(map(int, list(counts.keys())[0]))

def start_real_ibm_job(num_qubits: int) -> bytes:
    """Starts a job on a real IBM Quantum computer and returns the job ID."""
    service = QiskitRuntimeService()
    backend = service.least_busy(operational=True, simulator=False)

    circuit = QuantumCircuit(num_qubits, num_qubits)
    circuit.h(range(num_qubits))
    circuit.measure(range(num_qubits), range(num_qubits))

    job = service.run(backend=backend, circuits=[circuit])
    return json.dumps({"job_id": job.job_id()}).encode('utf-8')

def get_job_status(job_id: str) -> str:
    """Retrieve the status of a quantum job using IBM Qiskit Runtime."""
    service = QiskitRuntimeService()
    job = service.job(job_id)
    return job.status().value

def get_job_result(job_id: str) -> list[int]:
    """Retrieve the result of a quantum job using IBM Qiskit Runtime."""
    service = QiskitRuntimeService()
    job = service.job(job_id)
    result = job.result()
    counts = result.get_counts()

    return list(map(int, list(counts.keys())[0]))

def start_serial_connection(baudrate: int = 9600) -> None | Serial | Serial:
    port = find_arduino_port()
    if not port:
        print("No Arduino found. retrying...")
        start_serial_connection(baudrate)
        return None

    arduino = Serial(port, baudrate, timeout=1)
    time.sleep(2)  # Allow time for connection establishment
    print(f"Connected to {port}")
    return arduino


def execute_command(data: str) -> bytes | None:
    command = json.loads(data)
    if command["action"] == "start_job":
        return start_job_command(command.get("num_qubits", 1))
    elif command["action"] == "configure_ibm":
        return configure_ibm_token(command["token"])
    elif command["action"] == "get_job_status":
        status = get_job_status(command["job_id"])
        return json.dumps({"job_status": status}).encode('utf-8')
    elif command["action"] == "get_job_result":
        result = get_job_result(command["job_id"])
        return json.dumps({"job_result": result}).encode('utf-8')


def start_job_command(num_qubits: int) -> bytes:
    result = generate_superposition_qubits_simulated(num_qubits)
    response = json.dumps({"job_result": result})
    return response.encode('utf-8')


def send_response(arduino: Serial, data: bytes) -> None:
    print("Response: ", data)
    arduino.reset_input_buffer()
    arduino.write(data)


def process_serial_commands(arduino: Serial) -> None:
    while True:
        if arduino.in_waiting > 0:
            data = arduino.readline().decode('utf-8').strip()

            try:
                response = execute_command(data)
                if response:
                    send_response(arduino, response)

            except json.JSONDecodeError:
                arduino.write(json.dumps({"error": "Invalid JSON format"}).encode('utf-8'))


if __name__ == "__main__":
    serial_connection = start_serial_connection()
    process_serial_commands(serial_connection)
