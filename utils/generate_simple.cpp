#include <iostream>
#include <fstream>

// Simplified version without protobuf - Generates a basic binary file
int main() {
    std::ofstream output("configs/simulated_plcs.pb", std::ios::binary);
    
    if (!output) {
        std::cerr << "❌ Erro ao criar arquivo de configuração" << std::endl;
        return 1;
    }
    
    // Example data in simple binary format
    const char test_data[] = {
        0x12, 0x34, 0x01, 0x01, 0x00, 0x19, 0x00, 0x64, 0x01, 0xF4,
        0x00, 0xDC, 0x00, 0x32, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00,
        0x00, 0x00
    };
    
    output.write(test_data, sizeof(test_data));
    output.close();
    
    std::cout << "✅ Arquivo de configuração PLC gerado: configs/simulated_plcs.pb" << std::endl;
    std::cout << "📊 Tamanho: " << sizeof(test_data) << " bytes" << std::endl;
    
    return 0;
}