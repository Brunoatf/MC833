import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scapy.all import rdpcap, IP, ICMP
import os

def analisar_pcap(caminho_arquivo):
    pacotes = rdpcap(caminho_arquivo)

    pacotes_icmp = [pkt for pkt in pacotes if IP in pkt and ICMP in pkt]
    
    if not pacotes_icmp:
        print(f"Nenhum pacote ICMP encontrado em {caminho_arquivo}")
        return

    dados = {
        "tempo": [],
        "src": [],
        "dst": [],
        "tamanho": []
    }

    for pkt in pacotes_icmp:
        dados["tempo"].append(pkt.time)
        dados["src"].append(pkt[IP].src)
        dados["dst"].append(pkt[IP].dst)
        dados["tamanho"].append(len(pkt))  # Tamanho do pacote

    df = pd.DataFrame(dados)

    total_pacotes = len(df)

    duracao = df["tempo"].iloc[-1] - df["tempo"].iloc[0]
    total_bytes = df["tamanho"].sum()
    throughput = total_bytes / duracao if duracao > 0 else 0

    df["intervalo"] = df["tempo"].diff()
    intervalo_medio = df["intervalo"].mean()

    print(f"--- Análise de {caminho_arquivo} ---")
    print(f"Total de pacotes ICMP: {total_pacotes}")
    print(f"IPs de origem únicos: {df['src'].unique()}")
    print(f"IPs de destino únicos: {df['dst'].unique()}")
    print(f"Throughput médio: {throughput:.2f} bytes/s")
    print(f"Intervalo médio entre pacotes: {intervalo_medio:.6f} s\n")

    nome_base = os.path.basename(caminho_arquivo).replace(".pcap", "")
    
    plt.figure(figsize=(10, 6))
    plt.plot(df["tempo"], df["tamanho"], marker='o')
    plt.title(f"Tamanho dos Pacotes ao Longo do Tempo - {nome_base}")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Tamanho do Pacote (bytes)")
    plt.grid(True)
    plt.savefig(f"{nome_base}_tamanho_vs_tempo.png")
    plt.close()

    plt.figure(figsize=(10, 6))
    plt.hist(df["intervalo"].dropna(), bins=30, color='skyblue', edgecolor='black')
    plt.title(f"Histograma de Intervalos entre Pacotes - {nome_base}")
    plt.xlabel("Intervalo (s)")
    plt.ylabel("Frequência")
    plt.grid(True)
    plt.savefig(f"{nome_base}_hist_intervalos.png")
    plt.close()

arquivos_pcap = ["h1h3.pcap", "h2h4.pcap"]

for arquivo in arquivos_pcap:
    if os.path.exists(arquivo):
        analisar_pcap(arquivo)
    else:
        print(f"Arquivo não encontrado: {arquivo}")
