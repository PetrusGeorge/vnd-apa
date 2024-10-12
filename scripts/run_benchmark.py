import subprocess
import os
import pandas as pd
from tempfile import TemporaryFile
from concurrent.futures import ThreadPoolExecutor, as_completed

def process_iteration(instance, instance_dir, output_folder):

    instance_path = f"{instance_dir}/{instance}.txt"

    command = ["./result/bin/apa", instance_path, "-i", "30", "-ils", "120", "--benchmark"]

    with TemporaryFile() as tempf:
        subprocess.run( command,
            stdout=tempf,
        )

        tempf.seek(0)
        text = str(tempf.read().decode("utf-8")).split("\n")
        time = objective_value = construction_time = construction_value = ""
        for line in text:
            if line.find("Final cost: ") != -1:
                time = line.split(": ")[1]
            if line.find("Time: ") != -1:
                objective_value = line.split()[1]
            if line.find("Construction mean time: ") != -1:
                construction_time = line.split()[1]
            if line.find("Construction mean value: ") != -1:
                construction_value = line.split()[1]

        return (instance, time, objective_value, construction_time, construction_value)

def main():

    num_threads = 20 # number of threads used
    num_iterations = 10

    # File with the hashes of the instances that will be executed by the script
    instance_hashes = "instances"

    instance_dir = "instances/"
    output_file = "benchmark/result.csv"

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # compile
    subprocess.run(["just", "build"])

    with open(output_file, "w") as csv_header:
        csv_header.write("instance,otimo,guloso_valor,guloso_tempo,guloso_gap,ils_valor,ils_tempo,ils_gap\n")
    
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        with open(instance_hashes) as f:
            hashes = f.readlines()
            futures = [executor.submit(process_iteration, instance.strip(), instance_dir,
                      output_folder, id) 
                      for instance in hashes for id in range(num_iterations)]

            for future in as_completed(futures):
                with open(output_file, "a") as output_csv:
                    instance, time, objective_value = future.result()
                    output_csv.write(f"{instance},{time},{objective_value}\n")

            df = pd.read_csv(output_file)

            mean_df = df.groupby('instance_hash').agg({
                'time': 'mean',
                'objective_value': 'mean'
            }).reset_index()
            mean_df.columns = ['instance_hash', 'mean_time', 'mean_objective_value']

            # Rewrite the file
            mean_df.to_csv(output_file, index=False)

if __name__ == "__main__":
    main()

