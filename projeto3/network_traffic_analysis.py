import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import mean_squared_error
from sklearn.neural_network import MLPRegressor
import statsmodels.api as sm
import os
import random

# Set random seeds for reproducibility
np.random.seed(42)
random.seed(42)
os.environ['PYTHONHASHSEED'] = '42'

# Function to create sequences for time series prediction
def create_sequences(data, seq_len):
    X, y = [], []
    for i in range(len(data) - seq_len):
        X.append(data[i:i+seq_len])
        y.append(data[i+seq_len])
    return np.array(X), np.array(y)

# Function to train and evaluate neural network model
def train_evaluate_model(X_train, y_train, X_test, y_test):
    # Reshape X for MLPRegressor (samples, features)
    X_train_reshaped = X_train.reshape(X_train.shape[0], -1)
    X_test_reshaped = X_test.reshape(X_test.shape[0], -1)
    
    # Build neural network model
    model = MLPRegressor(
        hidden_layer_sizes=(50,),
        activation='relu',
        solver='adam',
        max_iter=200,
        random_state=42,
        early_stopping=True
    )
    
    # Train model
    model.fit(X_train_reshaped, y_train.ravel())
    
    # Make predictions
    train_predict = model.predict(X_train_reshaped).reshape(-1, 1)
    test_predict = model.predict(X_test_reshaped).reshape(-1, 1)
    
    # Calculate MSE
    train_mse = mean_squared_error(y_train, train_predict)
    test_mse = mean_squared_error(y_test, test_predict)
    
    return model, train_predict, test_predict, train_mse, test_mse

# Main function
def main():
    # 1. Load and prepare data
    print("Loading data...")
    df = pd.read_parquet('200701011400.parquet')
    
    # Convert timestamp to datetime if needed
    df['timestamp'] = pd.to_datetime(df['timestamp'])
    
    # 2. Exploratory Data Analysis (EDA)
    print("\n--- Exploratory Data Analysis ---")
    
    # Basic statistics
    print("\nBasic statistics for packet size:")
    print(df['size'].describe())
    
    # Create a directory for saving figures
    os.makedirs('figures', exist_ok=True)
    
    # Distribution of packet sizes
    plt.figure(figsize=(10, 6))
    plt.hist(df['size'], bins=50)
    plt.title('Distribution of Packet Sizes')
    plt.xlabel('Size (bytes)')
    plt.ylabel('Frequency')
    plt.savefig('figures/packet_size_distribution.png')
    
    # Distribution by protocol type
    plt.figure(figsize=(10, 6))
    df['type'].value_counts().plot(kind='bar')
    plt.title('Distribution of Protocol Types')
    plt.xlabel('Protocol')
    plt.ylabel('Count')
    plt.savefig('figures/protocol_distribution.png')
    
    # Create time series of bytes per second
    print("\nCreating time series of bytes per second...")
    df['second'] = df['timestamp'].dt.floor('s')
    traffic_per_sec = df.groupby('second')['size'].sum().reset_index()
    traffic_per_sec.rename(columns={'size': 'bytes'}, inplace=True)
    
    # Plot time series
    plt.figure(figsize=(15, 6))
    plt.plot(traffic_per_sec['second'], traffic_per_sec['bytes'])
    plt.title('Network Traffic (Bytes per Second)')
    plt.xlabel('Time')
    plt.ylabel('Bytes')
    plt.grid(True)
    plt.savefig('figures/traffic_time_series.png')
    
    # Separate by protocol
    tcp_df = df[df['type'] == 'TCP']
    udp_df = df[df['type'] == 'UDP']
    
    tcp_per_sec = tcp_df.groupby('second')['size'].sum().reset_index()
    udp_per_sec = udp_df.groupby('second')['size'].sum().reset_index()
    
    # Plot TCP vs UDP
    plt.figure(figsize=(15, 6))
    plt.plot(tcp_per_sec['second'], tcp_per_sec['size'], label='TCP')
    plt.plot(udp_per_sec['second'], udp_per_sec['size'], label='UDP')
    plt.title('TCP vs UDP Traffic')
    plt.xlabel('Time')
    plt.ylabel('Bytes')
    plt.legend()
    plt.grid(True)
    plt.savefig('figures/tcp_udp_comparison.png')
    
    # 3. Prepare data for prediction
    print("\n--- Preparing data for prediction ---")
    
    # Normalize the data
    scaler = MinMaxScaler(feature_range=(0, 1))
    scaled_data = scaler.fit_transform(traffic_per_sec[['bytes']])
    
    # Test different look-back windows
    look_back_values = [10, 20, 30]
    results = {}
    
    for look_back in look_back_values:
        print(f"\nTraining with look_back = {look_back}")
        
        # Create sequences
        X, y = create_sequences(scaled_data, look_back)
        
        # Split into train and test sets
        train_size = int(len(X) * 0.8)
        X_train, X_test = X[:train_size], X[train_size:]
        y_train, y_test = y[:train_size], y[train_size:]
        
        # Train and evaluate model
        model, train_predict, test_predict, train_mse, test_mse = train_evaluate_model(
            X_train, y_train, X_test, y_test
        )
        
        # Store results
        results[look_back] = {
            'model': model,
            'train_mse': train_mse,
            'test_mse': test_mse,
            'test_predict': test_predict,
            'y_test': y_test
        }
        
        print(f"Train MSE: {train_mse:.6f}")
        print(f"Test MSE: {test_mse:.6f}")
        
        # Plot predictions vs actual
        # Invert predictions
        test_predict_inv = scaler.inverse_transform(test_predict)
        y_test_inv = scaler.inverse_transform(y_test.reshape(-1, 1))
        
        # Create a time index for the test data
        test_time_idx = traffic_per_sec['second'].iloc[train_size + look_back:train_size + look_back + len(test_predict)]
        
        plt.figure(figsize=(15, 6))
        plt.plot(test_time_idx, y_test_inv, label='Actual')
        plt.plot(test_time_idx, test_predict_inv, label='Predicted')
        plt.title(f'Network Traffic Prediction (look_back={look_back})')
        plt.xlabel('Time')
        plt.ylabel('Bytes')
        plt.legend()
        plt.grid(True)
        plt.savefig(f'figures/prediction_vs_actual_lookback_{look_back}.png')
    
    # 4. Compare results for different look-back windows
    print("\n--- Comparison of Different Look-back Windows ---")
    mse_values = {k: v['test_mse'] for k, v in results.items()}
    
    # Create a table of MSE values
    mse_df = pd.DataFrame({
        'Look-back Window': list(mse_values.keys()),
        'Test MSE': list(mse_values.values())
    })
    print("\nMSE values for different look-back windows:")
    print(mse_df)
    
    # Plot MSE comparison
    plt.figure(figsize=(10, 6))
    plt.bar(mse_df['Look-back Window'].astype(str), mse_df['Test MSE'])
    plt.title('MSE Comparison for Different Look-back Windows')
    plt.xlabel('Look-back Window')
    plt.ylabel('Test MSE')
    plt.grid(True, axis='y')
    plt.savefig('figures/mse_comparison.png')
    
    # 5. Additional analysis - Autocorrelation
    print("\n--- Autocorrelation Analysis ---")
    
    # Calculate autocorrelation
    bytes_series = traffic_per_sec['bytes']
    fig = plt.figure(figsize=(12, 6))
    ax1 = fig.add_subplot(211)
    sm.graphics.tsa.plot_acf(bytes_series, lags=60, ax=ax1)
    ax2 = fig.add_subplot(212)
    sm.graphics.tsa.plot_pacf(bytes_series, lags=60, ax=ax2)
    plt.tight_layout()
    plt.savefig('figures/autocorrelation.png')
    
    print("\nAnalysis complete. Results and figures saved.")

if __name__ == "__main__":
    main()