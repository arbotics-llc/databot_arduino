// See https://tech.shutterstock.com/rickshaw/

var clock_tick_count = 1;

var dashboard = {

	visualA:{

	},

	visualB:{

	},

}

google.charts.load('current', {'packages':['corechart','gauge']});

google.charts.setOnLoadCallback(on_google_lib_loaded);

function on_google_lib_loaded() {

	drawChart();

}

function paint() {

	dashboard.visualA.chart.draw(dashboard.visualA.data, dashboard.visualA.options);
	dashboard.visualB.chart.draw(dashboard.visualB.data, dashboard.visualB.options);

}

function drawChart() {

	dashboard.visualA.data = google.visualization.arrayToDataTable([
			['Time', 'CO2'],
			['0', 0]
		]);

	dashboard.visualA.options = { curveType: 'function', legend: { position: 'bottom' }};

	dashboard.visualA.chart = new google.visualization.LineChart(document.getElementById('chart'));

	dashboard.visualB.data = google.visualization.arrayToDataTable([['Label', 'Value'],['PSI', 0]]);

	dashboard.visualB.options = { width:700, height:520, min:0, max:1200, minorTicks:50 };

	dashboard.visualB.options.redFrom = dashboard.visualB.options.max * 0.80;
	dashboard.visualB.options.redTo = dashboard.visualB.options.max;
	dashboard.visualB.options.yellowFrom = dashboard.visualB.options.max * 0.60;
	dashboard.visualB.options.yellowTo = dashboard.visualB.options.max * 0.80;

	dashboard.visualB.chart = new google.visualization.Gauge(document.getElementById('chart_div'));

	paint();

	setInterval(function() {

		on_new_data_received(`m${clock_tick_count.toFixed(3)};c${Math.floor(Math.random()*109+400).toFixed(3)};a${Math.floor(Math.random()*1200).toFixed(3)};`);

		++clock_tick_count;

	}, 500);

}

function process_databot_data (databot_data) {

	dashboard.visualA.data.addRows([[databot_data[0].toString(), databot_data[1]]]);

	dashboard.visualB.data.setValue(0, 1, databot_data[2]);

	paint();

}

function on_new_data_received (databot_stream) {

	var ax = databot_stream.split(";").slice(0,-1);

	var bx = ax.map(p => parseFloat(p.substring(1)));

	process_databot_data(bx);

}
