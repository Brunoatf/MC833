
# Analisador de Pacotes ICMP (`package_analyzer.py`)

Este script analisa arquivos `.pcap` contendo pacotes ICMP (gerados via Wireshark e Mininet), extraindo métricas importantes de rede e gerando gráficos ilustrativos.

## O que o script faz:
- Lê os arquivos `h1h3.pcap` e `h2h4.pcap`
- Filtra pacotes ICMP
- Extrai:
  - Endereços IP de origem e destino
  - Contagem total de pacotes
  - Throughput médio (bytes/segundo)
  - Intervalo médio entre pacotes (segundos)
- Gera gráficos:
  - Tamanho dos pacotes ao longo do tempo
  - Histograma dos intervalos entre pacotes

## Pré-requisitos

Instale as bibliotecas necessárias com:

```bash
pip install scapy pandas numpy matplotlib nest_asyncio
```

## Como rodar

1. Certifique-se de ter os arquivos `h1h3.pcap` e `h2h4.pcap` na mesma pasta do script.
2. Execute o script:

```bash
python package_analyzer.py
```

## Saídas geradas

Após a execução, você verá no diretório atual:
- Arquivos de imagem `.png` com os gráficos gerados:
  - `h1h3_tamanho_vs_tempo.png`
  - `h1h3_hist_intervalos.png`
  - `h2h4_tamanho_vs_tempo.png`
  - `h2h4_hist_intervalos.png`

Além disso, o terminal exibirá os dados extraídos de cada arquivo `.pcap`.
