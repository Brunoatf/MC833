# Previsão de Tráfego de Rede com Redes Neurais Recorrentes

Este projeto implementa um sistema de previsão de tráfego de rede utilizando redes neurais recorrentes (LSTM) para analisar e prever o volume de tráfego segundo a segundo.

## Estrutura do Projeto

- `network_traffic_analysis.py`: Script para análise exploratória e modelagem básica
- `keras_lstm_implementation.py`: Implementa especificamente um modelo LSTM usando TensorFlow/Keras para prever tráfego de rede, testando diferentes tamanhos de janela (10, 20, 30 segundos) e comparando seus desempenhos.
- `figures/`: Diretório contendo gráficos e visualizações gerados pelos scripts

## Requisitos

- Python 3.8+
- pandas
- numpy
- matplotlib
- scikit-learn
- statsmodels
- pyarrow
- tensorflow (para a implementação com Keras)

## Configuração do Ambiente

1. Crie um ambiente virtual:
   ```
   python3 -m venv venv
   ```

2. Ative o ambiente virtual:
   ```
   source venv/bin/activate  # Linux/Mac
   venv\Scripts\activate     # Windows
   ```

3. Instale as dependências básicas:
   ```
   pip install pandas pyarrow matplotlib scikit-learn statsmodels
   ```

4. Para a implementação com TensorFlow/Keras, instale:
   ```
   pip install tensorflow
   ```

## Executando o Projeto

obs: Faça o download do arquivo 200701011400.parquet antes

1. Para realizar a análise exploratória e modelagem básica:
   ```
   python network_traffic_analysis.py
   ```

2. Para executar a implementação LSTM com TensorFlow/Keras:
   ```
   python keras_lstm_implementation.py
   ```



## Resultados

Os resultados da análise e modelagem são salvos no diretório `figures/`, incluindo:
- Distribuição de protocolos
- Série temporal de tráfego
- Comparação entre tráfego TCP e UDP
- Previsões vs. valores reais para diferentes tamanhos de janela
- Comparação de MSE entre diferentes modelos
