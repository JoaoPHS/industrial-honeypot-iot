# Script de Limpeza para Publicação no GitHub
# Remove arquivos de compilação e temporários

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Limpando Projeto para GitHub         " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$itemsToRemove = @(
    "build",
    "logs",
    "*.o",
    "*.a",
    "*.so",
    "*.exe",
    "*.out",
    "CMakeCache.txt",
    "cmake_install.cmake",
    "Makefile",
    "*.log",
    "*.tmp",
    "*.bak",
    "test-powershell.ps1",
    "start-server.ps1",
    "PUBLICAR.md"
)

$removedCount = 0
$failedCount = 0

foreach ($item in $itemsToRemove) {
    Write-Host "[Procurando] $item..." -ForegroundColor Yellow
    
    if ($item -like "*\*" -or $item -like "*.*") {
        # É um padrão de arquivo
        $files = Get-ChildItem -Path . -Filter $item -Recurse -ErrorAction SilentlyContinue
        foreach ($file in $files) {
            try {
                Remove-Item $file.FullName -Force -ErrorAction Stop
                Write-Host "  ✓ Removido: $($file.Name)" -ForegroundColor Green
                $removedCount++
            } catch {
                Write-Host "  ✗ Erro: $($file.Name)" -ForegroundColor Red
                $failedCount++
            }
        }
    } else {
        # É um diretório
        if (Test-Path $item) {
            try {
                Remove-Item $item -Recurse -Force -ErrorAction Stop
                Write-Host "  ✓ Removido: $item/" -ForegroundColor Green
                $removedCount++
            } catch {
                Write-Host "  ✗ Erro ao remover: $item" -ForegroundColor Red
                $failedCount++
            }
        } else {
            Write-Host "  - Não encontrado: $item" -ForegroundColor Gray
        }
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Limpeza Concluída!                   " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Itens removidos: $removedCount" -ForegroundColor Green
Write-Host "Falhas: $failedCount" -ForegroundColor $(if($failedCount -gt 0){"Red"}else{"Green"})
Write-Host ""
Write-Host "Arquivos mantidos:" -ForegroundColor Yellow
Write-Host "  ✓ Código fonte (server/)" -ForegroundColor White
Write-Host "  ✓ Headers (include/)" -ForegroundColor White
Write-Host "  ✓ CMakeLists.txt" -ForegroundColor White
Write-Host "  ✓ Dockerfile" -ForegroundColor White
Write-Host "  ✓ README.md e documentação" -ForegroundColor White
Write-Host "  ✓ Scripts PowerShell" -ForegroundColor White
Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor Cyan
Write-Host "  1. Revisar .gitignore" -ForegroundColor White
Write-Host "  2. git add ." -ForegroundColor White
Write-Host "  3. git commit -m 'Versão limpa para publicação'" -ForegroundColor White
Write-Host "  4. git push" -ForegroundColor White
Write-Host ""

