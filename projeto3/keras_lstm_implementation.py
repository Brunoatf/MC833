import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import mean_squared_error
import os
import random
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense
from tensorflow.keras.optimizers import Adam

# Definir sementes aleatórias para reprodutibilidade
np.random.seed(42)
random.seed(42)
os.environ['PYTHONHASHSEED'] = '42'
tf.random.set_seed(42)

# Função para criar sequências para LSTM
def create_sequences(data, seq_len):
    X, y = [], []
    for i in range(len(data) - seq_len):
        X.append(data[i:i+seq_len])
        y.append(data[i+seq_len])
    return np.array(X), np.array(y)

# Função para construir e treinar modelo LSTM
def build_train_lstm(X_train, y_train, X_test, y_test, look_back, epochs=10, batch_size=32):
    # Construir o modelo LSTM
    model = Sequential()
    model.add(LSTM(50, activation='relu', input_shape=(look_back, 1)))
    model.add(Dense(1))
    
    # Compilar o modelo
    model.compile(optimizer=Adam(learning_rate=0.001), loss='mse')
    
    # Treinar o modelo
    history = model.fit(
        X_train, y_train,
        epochs=epochs,
        batch_size=batch_size,
        validation_data=(X_test, y_test),
        verbose=1
    )
    
    # Fazer previsões
    train_predict = model.predict(X_train)
    test_predict = model.predict(X_test)
    
    # Calcular MSE
    train_mse = mean_squared_error(y_train, train_predict)
    test_mse = mean_squared_error(y_test, test_predict)
    
    return model, history, train_predict, test_predict, train_mse, test_mse

# Função principal
def main():
    # 1. Carregar e preparar dados
    print("Carregando dados...")
    df = pd.read_parquet('200701011400.parquet')
    
    # Converter timestamp para datetime se necessário
    df['timestamp'] = pd.to_datetime(df['timestamp'])
    
    # Criar série temporal de bytes por segundo
    print("\nCriando série temporal de bytes por segundo...")
    df['second'] = df['timestamp'].dt.floor('s')
    traffic_per_sec = df.groupby('second')['size'].sum().reset_index()
    traffic_per_sec.rename(columns={'size': 'bytes'}, inplace=True)
    
    # Criar diretório para salvar figuras
    os.makedirs('figures', exist_ok=True)
    
    # 2. Preparar dados para LSTM
    print("\n--- Preparando dados para LSTM ---")
    
    # Normalizar os dados
    scaler = MinMaxScaler(feature_range=(0, 1))
    scaled_data = scaler.fit_transform(traffic_per_sec[['bytes']])
    
    # Testar diferentes tamanhos de janela
    look_back_values = [10, 20, 30]
    results = {}
    
    for look_back in look_back_values:
        print(f"\nTreinando com look_back = {look_back}")
        
        # Criar sequências
        X, y = create_sequences(scaled_data, look_back)
        
        # Dividir em conjuntos de treino e teste
        train_size = int(len(X) * 0.8)
        X_train, X_test = X[:train_size], X[train_size:]
        y_train, y_test = y[:train_size], y[train_size:]
        
        # Remodelar entrada para [amostras, passos de tempo, características]
        X_train = X_train.reshape(X_train.shape[0], X_train.shape[1], 1)
        X_test = X_test.reshape(X_test.shape[0], X_test.shape[1], 1)
        
        # Construir e treinar modelo LSTM
        model, history, train_predict, test_predict, train_mse, test_mse = build_train_lstm(
            X_train, y_train, X_test, y_test, 
            look_back=look_back, 
            epochs=10,
            batch_size=32
        )
        
        # Armazenar resultados
        results[look_back] = {
            'model': model,
            'history': history,
            'train_mse': train_mse,
            'test_mse': test_mse,
            'test_predict': test_predict,
            'y_test': y_test
        }
        
        print(f"MSE de Treino: {train_mse:.6f}")
        print(f"MSE de Teste: {test_mse:.6f}")
        
        # Plotar histórico de treinamento
        plt.figure(figsize=(10, 6))
        plt.plot(history.history['loss'], label='Perda de Treino')
        plt.plot(history.history['val_loss'], label='Perda de Validação')
        plt.title(f'Perda do Modelo (look_back={look_back})')
        plt.ylabel('Perda')
        plt.xlabel('Época')
        plt.legend()
        plt.grid(True)
        plt.savefig(f'figures/keras_loss_history_lookback_{look_back}.png')
        
        # Plotar previsões vs valores reais
        # Inverter previsões
        test_predict_inv = scaler.inverse_transform(test_predict)
        y_test_inv = scaler.inverse_transform(y_test.reshape(-1, 1))
        
        # Criar índice de tempo para os dados de teste
        test_time_idx = traffic_per_sec['second'].iloc[train_size + look_back:train_size + look_back + len(test_predict)]
        
        plt.figure(figsize=(15, 6))
        plt.plot(test_time_idx, y_test_inv, label='Real')
        plt.plot(test_time_idx, test_predict_inv, label='Previsto')
        plt.title(f'Previsão de Tráfego de Rede com Keras LSTM (look_back={look_back})')
        plt.xlabel('Tempo')
        plt.ylabel('Bytes')
        plt.legend()
        plt.grid(True)
        plt.savefig(f'figures/keras_prediction_lookback_{look_back}.png')
    
    # 3. Comparar resultados para diferentes tamanhos de janela
    print("\n--- Comparação de Diferentes Tamanhos de Janela ---")
    mse_values = {k: v['test_mse'] for k, v in results.items()}
    
    # Criar tabela de valores MSE
    mse_df = pd.DataFrame({
        'Tamanho de Janela': list(mse_values.keys()),
        'MSE de Teste': list(mse_values.values())
    })
    print("\nValores de MSE para diferentes tamanhos de janela:")
    print(mse_df)
    
    # Plotar comparação de MSE
    plt.figure(figsize=(10, 6))
    plt.bar(mse_df['Tamanho de Janela'].astype(str), mse_df['MSE de Teste'])
    plt.title('Comparação de MSE do Keras LSTM para Diferentes Tamanhos de Janela')
    plt.xlabel('Tamanho de Janela')
    plt.ylabel('MSE de Teste')
    plt.grid(True, axis='y')
    plt.savefig('figures/keras_lstm_mse_comparison.png')
    
    print("\nAnálise do Keras LSTM completa. Resultados e figuras salvas.")

if __name__ == "__main__":
    main()