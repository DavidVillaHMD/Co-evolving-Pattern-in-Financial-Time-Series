# temp_data = [3.77, 3.72, 3.67, 3.65, 3.69, 6.72, 3.64, 3.67, 3.82, 3.82, 3.75]
# temp_data = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 ]
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import time

start = time.time()
root_dir = os.getcwd()
list_of_stocks = np.loadtxt('./input/sector.txt', dtype=str)
# list_of_stocks = ['000031']
final_results = []
num_recorder = open('./output/estate_num.txt', 'w')


def get_qm(temp_data, degree_of_freedom=3):
    '''
    :param temp_data: Time Series
    :param degree_of_freedom: degree_of_freedom of chi-square distribution
    :return: whether input time data is a white noise series according to the preset threshold.
    '''
    # temp_data = list(range(1,12))
    chi_square_coefficient = [0.00393, 0.103, 0.352, 0.711, 1.145]  # 95% confidence
    temp_data = list(temp_data)
    rho = []    # vector of auto-correlations
    variance = 0
    total_temp = 0
    for x in temp_data:
        total_temp += x
    mean = total_temp / len(temp_data)
    for x in temp_data:
        variance += (x - mean) * (x - mean)
    variance = variance / len(temp_data)
    if variance == 0:
        return True
    for x in range(1, len(temp_data)):
        temp = 0
        for y in range(0, len(temp_data) - x):
            temp += ((temp_data[y] - mean)*(temp_data[y+x] - mean))
        temp = temp / len(temp_data) / variance
        rho.append(temp)
    # lb = 0  # Ljung-Box QM
    bp = 0  # Box-Pierce QM
    for m in range(degree_of_freedom):
        # lb += rho[m] * rho[m] / (len(temp_data) - m - 1)
        bp += (rho[m] * rho[m])
    bp *= len(temp_data)
    #  print(bp)
    # lb = lb*len(temp_data)*(len(temp_data)+2)
    # print(lb)
    # 0.352
    # print("{}    {}".format(len(temp_data), bp))
    if bp > chi_square_coefficient[degree_of_freedom - 1]:
        # lb is definitely bigger than bp, we can change 'bp' into 'lb' to lb mode.
        return False
    else:
        return True


def read_raw_data(name):
    result = pd.read_csv('./input/'+name+'.csv', encoding='gbk')
    result['Date'] = pd.to_datetime(result['Date'], format="%Y-%m-%d")
    # a.plot(x='Date', y='Adj Close', label=list_of_stocks[i], ax=ax)
    return result


def finding_ending_of_stationary(df, start_index, change_threshold=0.1):
    '''
    :param df: a dataframe
    :param start_index:
    :param change_threshold: the max change rate, default value is 0.1
    :return: the index of the last data which can hold the stationary condition
    '''
    upbound = df.iat[start_index, 6] * (1 + change_threshold)
    lowbound = df.iat[start_index, 6] * (1 - change_threshold)
    i = start_index + 1
    temp = df.iat[start_index, 6]
    while i < df.shape[0]:
        if lowbound < df.iat[i, 6] < upbound and abs((df.iat[i, 6] - temp) / temp) <= change_threshold:
            temp = df.iat[i, 6]
            i += 1
            continue
        else:
            break
    return i  # Actually data with index i will not be recognized as a member of stationary series


def main_process(min_wn_length, name_of_stock):
    time_series = read_raw_data(name_of_stock)
    i = 0  # start index
    while i <= time_series.shape[0] - min_wn_length:
        nexti = finding_ending_of_stationary(time_series, i)
        if nexti - i <= min_wn_length:
            i += 1
            continue
        else:
            while nexti >= i + min_wn_length:
                if get_qm(time_series['Adj Close'][i:nexti]):
                    print("{} -- {}".format(i, nexti))
                    final_results.append([i, nexti])
                    i = nexti
                    break
                else:
                    nexti -= 1
        i += 1


def output_for_single_section(order, stock_df, start, end, stock_code_output):
    '''
    This function is to output data of real prices in designed formula.
    :param order: the order of this section in this stock
    :param stock_df: dataframe of a stock
    :param start: the start order of the section
    :param end: the end order of the section, which is not a part of result
    :param stock_code_output: input as a parameter just for convenience of output
    :return: True for successful output, False for incorrect, also output a txt file in './output'
    '''
    if end - start <= 5:
        return False
    f1 = open('./output/'+stock_code_output+'_'+str(order)+'.txt', 'w')
    f2 = open('./output/'+stock_code_output+'_'+str(order)+'_DATE.txt', 'w')
    for x in stock_df['Adj Close'][start:end]:
        print('%.3f' % x, file=f1)
    for x in stock_df['Date'][start:end]:
        x = str(x)[:10]
        print(x, file=f2)
    f1.close()
    f2.close()
    return True


for stock_code in list_of_stocks:
    print(stock_code)
    main_process(10, stock_code)  # 10 is changeable
    a = read_raw_data(stock_code)
    total = 0
    temp_start = 0
    order = 1
    for xxx in final_results:
        total += xxx[1] - xxx[0]
        if output_for_single_section(order, a, temp_start, xxx[0], stock_code):
            order += 1
        temp_start = xxx[1]
    output_for_single_section(order, a, temp_start, a.shape[0], stock_code)
    print('{}/{}'.format(total, a.shape[0]))
    final_results = []
    print(order, file=num_recorder)
num_recorder.close()
end = time.time()
print(end-start)
'''    # VISIBLE SESSION
    a.plot(y='Adj Close', label=list_of_stocks[0])
    total = 0
    for xxx in final_results:
        plt.axvspan(xxx[0], xxx[1]-1, color='red', alpha=0.5)
        total += xxx[1] - xxx[0]
    print('{}/{}'.format(total, a.shape[0]))
    plt.show()
'''