#!/bin/bash

mkdir -p output

erros=()
ooms=()

for arquivo_completo in ../instances/realistic-instances/*; do
    if [ -f "$arquivo_completo" ]; then

        nome_arquivo=$(basename "$arquivo_completo")
        arquivo_saida="output/${nome_arquivo}.out"

        if [ ! -f "$arquivo_saida" ]; then

            echo "========================================"
            echo "Processando: $nome_arquivo"
            echo "========================================"

            ./pl "$arquivo_completo" "$arquivo_saida"

            status=$?

            if [ $status -ne 0 ]; then

                echo "❌ Falha na instância: $nome_arquivo"
                echo "Código de saída: $status"

                erros+=("$nome_arquivo (exit code $status)")

                # SIGKILL normalmente é 137 = 128 + 9
                if [ $status -eq 137 ]; then
                    echo "⚠️ Possível OOM detectado!"
                    ooms+=("$nome_arquivo")
                fi
            else
                echo "✅ Finalizado: $nome_arquivo"
            fi

        else
            echo "⏭️ Já processado (pulando): $nome_arquivo"
        fi
    fi
done


echo ""
echo "========================================"
echo "RESUMO FINAL"
echo "========================================"


if [ ${#erros[@]} -eq 0 ]; then

    echo "✅ Nenhum erro encontrado."

else

    echo ""
    echo "❌ Instâncias com erro (${#erros[@]}):"

    for erro in "${erros[@]}"; do
        echo "  - $erro"
    done

    printf "%s\n" "${erros[@]}" > output/erros.txt

    echo ""
    echo "Lista completa salva em:"
    echo "output/erros.txt"

fi


if [ ${#ooms[@]} -gt 0 ]; then

    echo ""
    echo "========================================"
    echo "⚠️ POSSÍVEIS OOM (${#ooms[@]})"
    echo "========================================"

    for oom in "${ooms[@]}"; do
        echo "  - $oom"
    done

    printf "%s\n" "${ooms[@]}" > output/ooms.txt

    echo ""
    echo "Lista salva em:"
    echo "output/ooms.txt"

fi


echo ""
echo "Processamento completo." 