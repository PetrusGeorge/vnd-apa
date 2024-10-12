import subprocess
import os
import pandas as pd
from tempfile import TemporaryFile
from concurrent.futures import ThreadPoolExecutor, as_completed

def process_iteration(instance, instance_dir):

    instance,optimal = instance.split(":")
    instance_path = f"{instance_dir}/{instance}.txt"

    command = ["./result/bin/apa", instance_path, "-i", "50", "-ils", "180", "--benchmark"]

    with TemporaryFile() as tempf:
        subprocess.run( command,
            stdout=tempf,
        )

        tempf.seek(0)
        text = str(tempf.read().decode("utf-8")).split("\n")
        time = objective_value = construction_time = construction_value = ""
        for line in text:
            if line.find("Final cost: ") != -1:
                objective_value = line.split(": ")[1]
            if line.find("Time: ") != -1:
                time = line.split(": ")[1]
            if line.find("Construction mean time: ") != -1:
                construction_time = line.split(": ")[1]
            if line.find("Construction mean value: ") != -1:
                construction_value = line.split(": ")[1].strip()

        return (instance, optimal, time, objective_value, construction_time, construction_value)

def main():

    num_threads = 6 # number of threads used
    num_iterations = 10

    # File with the hashes of the instances that will be executed by the script
    instance_names = "instances-names"

    instance_dir = "instances/"
    output_file = "result.csv"

    # compile
    subprocess.run(["nix", "build"])

    with open(output_file, "w") as csv_header:
        csv_header.write("instance,otimo,guloso_best,guloso_media,guloso_tempo(ms),guloso_gap,ils_best,ils_media,ils_tempo(s),ils_gap\n")
    
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        with open(instance_names) as f:
            name = f.readlines()
            futures = [executor.submit(process_iteration, instance.strip(), instance_dir) 
                      for instance in name for _ in range(num_iterations)]

            for future in as_completed(futures):
                with open(output_file, "a") as output_csv:
                    instance, optimal, time, objective_value, construction_time, construction_value = future.result()
                    try:
                        construction_gap = 100.0 * float(construction_value) / float(optimal) - 100
                    except ZeroDivisionError:
                        construction_gap = 0
                    try:
                        ils_gap = 100.0 * float(objective_value) / float(optimal) - 100
                    except ZeroDivisionError:
                            ils_gap = 0

                    output_csv.write(f"{instance},{optimal},{construction_value},{construction_value},{float(construction_time)*1000},{construction_gap},{objective_value},{objective_value},{time},{ils_gap}\n")

            df = pd.read_csv(output_file)

            mean_df = df.groupby('instance').agg({
                'otimo': 'min',
                'guloso_best': 'min',
                'guloso_media': 'mean',
                'guloso_tempo(ms)': 'mean',
                'guloso_gap': 'mean',
                'ils_best': 'min',
                'ils_media': 'mean',
                'ils_tempo(s)': 'mean',
                'ils_gap': 'mean',
            }).reset_index()
            mean_df.columns = ['instance', 'otimo', 'guloso_best', 'gulosos_valor', 'guloso_tempo(ms)', 'guloso_gap', 'ils_best', 'ils_media', 'ils_tempo(s)' ,'ils_gap']

            format_float = lambda x: '{:.3f}'.format(x).rstrip('0').rstrip('.')
            columns_to_format = ['gulosos_valor', 'guloso_tempo(ms)', 'guloso_gap','ils_media', 'ils_tempo(s)', 'ils_gap']
            mean_df[columns_to_format] = mean_df[columns_to_format].map(format_float)

            # Rewrite the file
            mean_df.to_csv(output_file, index=False)

if __name__ == "__main__":
    main()

