<?php
$hostsFile = '/etc/gteam/dynamic/supervisor/server/config/hosts.json';
$resultsFile = '/etc/gteam/dynamic/supervisor/server/results/ping_results.json';

if (file_exists($hostsFile)) {

    $hosts = json_decode(file_get_contents($hostsFile), true);

    if (!is_array($hosts)) {

        $hosts = [];

    }

} else {

    $hosts = [];

}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    if (isset($_POST['name']) && isset($_POST['ip'])) {

        $hosts[] = ["name" => $_POST['name'], "ip" => $_POST['ip']];

    } elseif (isset($_POST['remove'])) {

        $index = $_POST['remove'];

        if (isset($hosts[$index])) {

            array_splice($hosts, $index, 1);

        }

    } elseif (isset($_POST['edit']) && isset($_POST['edit_name']) && isset($_POST['edit_ip'])) {

        $index = $_POST['edit'];

        if (isset($hosts[$index])) {

            $hosts[$index]['name'] = $_POST['edit_name'];
            $hosts[$index]['ip'] = $_POST['edit_ip'];

        }

    }

    file_put_contents($hostsFile, json_encode($hosts, JSON_PRETTY_PRINT));

    header("Location: index.php");

    exit;

}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dynamic.Supervisor</title>
    <link rel="stylesheet" href="css/style.css">
</head>
<body>

<h2>Manage Hosts</h2>

<form method="post">

    <input type="text" name="name" placeholder="Host Name" required>
    <input type="text" name="ip" placeholder="IP Address" required>
    <button type="submit">Add Host</button>

</form>

<table id="pingTable" border="1">

    <tr>

        <th>Name</th>
        <th>IP Address</th>
        <th>Actions</th>
        <th>Latency (ms)</th>
        <th>CPU Usage (%)</th>

    </tr>

    <?php foreach ($hosts as $index => $host): ?>

        <tr data-host-id="<?= htmlspecialchars($host['ip']) ?>">

            <td><?= htmlspecialchars($host['name']) ?></td>
            <td><?= htmlspecialchars($host['ip']) ?></td>

            <td>

                <div class="button-container">

                    <form method="post" style="display:inline;">

                        <input type="hidden" name="remove" value="<?= $index ?>">
                        <button type="submit">Remove</button>

                    </form>

                    <form method="post" style="display:inline;">

                        <input type="hidden" name="edit" value="<?= $index ?>">
                        <input type="text" name="edit_name" value="<?= htmlspecialchars($host['name']) ?>" required>
                        <input type="text" name="edit_ip" value="<?= htmlspecialchars($host['ip']) ?>" required>
                        <button type="submit">Edit</button>

                    </form>

                </div>

            </td>

            <td id="latency-<?= htmlspecialchars($host['ip']) ?>" class="latency-unknown latency-loading">Loading...</td>
            <td id="cpu-<?= htmlspecialchars($host['ip']) ?>" class="cpu-unknown">Loading...</td>

        </tr>

    <?php endforeach; ?>

</table>

<script>

    setInterval(updateLatencyAndCpuUsage, 1000);
    updateLatencyAndCpuUsage();

    async function updateLatencyAndCpuUsage() {

        try {

            const response = await fetch('php/results.php');
            const data = await response.json();

            if (Array.isArray(data)) {

                data.forEach((host) => {

                    const hostIP = host.ip;
                    const latency = host.latency_ms;
                    const cpuUsage = host.cpu_usage;
                    const latencyCell = document.getElementById(`latency-${hostIP}`);
                    const cpuCell = document.getElementById(`cpu-${hostIP}`);

                    if (latencyCell) {

                        if (latency === null || latency === -1) {

                            latencyCell.textContent = 'Timeout';
                            latencyCell.className = 'color-high';

                        } else if (latency < 10) {

                            latencyCell.textContent = `${latency} ms`;
                            latencyCell.className = 'color-low';

                        } else {

                            latencyCell.textContent = `${latency} ms`;
                            latencyCell.className = 'color-medium';
                        }

                    }

                    if (cpuCell) {

                        if (isNaN(cpuUsage)) {

                            cpuCell.textContent = cpuUsage;
                            cpuCell.className = 'color-unknown';

                        } else {

                            const cpuValue = parseFloat(cpuUsage).toFixed(2);
                            cpuCell.textContent = `${cpuValue}%`;

                            if (cpuValue < 20) {

                                cpuCell.className = 'color-low';

                            } else if (cpuValue >= 20 && cpuValue <= 80) {

                                cpuCell.className = 'color-medium';

                            } else {

                                cpuCell.className = 'color-high';

                            }

                        }

                    }

                });

            } else {

                console.error("Error: Results data is not an array.");

            }

        } catch (error) {

            console.error("Error fetching results:", error);

        }

    }

</script>

</body>
</html>
