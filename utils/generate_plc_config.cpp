#include <iostream>
#include <fstream>
#include <string>
#include "../proto/plc_memory.pb.h" 

int main() {
    iot::honeypot::PLCMemory plc_memory;

    // Configure input/output regions as an example
    plc_memory.set_input_region("\x01\x00\x01\x01");
    plc_memory.set_output_region("\x00\x00\x00\x00");

    // Adds some holding registers (work_memory)
    (*plc_memory.mutable_work_memory())["0"] = 0x1234;
    (*plc_memory.mutable_work_memory())["1"] = 0x0101;
    (*plc_memory.mutable_work_memory())["2"] = 25;
    (*plc_memory.mutable_work_memory())["3"] = 100;
    (*plc_memory.mutable_work_memory())["4"] = 500;

    // Adds some holding registers (data_registers)
    (*plc_memory.mutable_data_registers())["0"] = 220;
    (*plc_memory.mutable_data_registers())["1"] = 50;

    // Saves to file
    std::ofstream output("configs/simulated_plcs.pb", std::ios::binary);
    if (!plc_memory.SerializeToOstream(&output)) {
        std::cerr << "❌ Erro ao salvar configuração PLC" << std::endl;
        return 1;
    }

    std::cout << "✅ Arquivo de configuração PLC gerado: configs/simulated_plcs.pb" << std::endl;
    return 0;
}