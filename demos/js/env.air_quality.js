// See https://tech.shutterstock.com/rickshaw/

var clock_tick_count = 1;

var dashboard = {

	chartA:{

	},

	chartB:{

	},

	chartC:{

	},

}

google.charts.load('current', {'packages':['gauge']});

google.charts.setOnLoadCallback(on_google_lib_loaded);

function on_google_lib_loaded() {

	drawChart();

}

function paint() {

	dashboard.chartA.chart.draw(dashboard.chartA.data,dashboard.chartA.options);
	dashboard.chartB.chart.draw(dashboard.chartB.data,dashboard.chartB.options);
	dashboard.chartC.chart.draw(dashboard.chartC.data,dashboard.chartC.options);

}

function drawChart() {

	dashboard.chartA.data = google.visualization.arrayToDataTable([['Label', 'Value'],['kPa', 0]]);
	dashboard.chartB.data = google.visualization.arrayToDataTable([['Label', 'Value'],['RH%', 0]]);
	dashboard.chartC.data = google.visualization.arrayToDataTable([['Label', 'Value'],['Celsius', 0]]);

	dashboard.chartA.options = { height:300, min:0, max:1200, minorTicks:40 };
	dashboard.chartB.options = { height:300, min:0, max:100, minorTicks:20 };
	dashboard.chartC.options = { height:300, min:-50, max:100, minorTicks:10 };
/*
	dashboard.chartA.options.redFrom = dashboard.chartA.options.max * 0.80;
	dashboard.chartA.options.redTo = dashboard.chartA.options.max;
	dashboard.chartA.options.yellowFrom = dashboard.chartA.options.max * 0.60;
	dashboard.chartA.options.yellowTo = dashboard.chartA.options.max * 0.80;
*/
	dashboard.chartA.chart = new google.visualization.Gauge(document.getElementById('chart1'));
	dashboard.chartB.chart = new google.visualization.Gauge(document.getElementById('chart2'));
	dashboard.chartC.chart = new google.visualization.Gauge(document.getElementById('chart3'));

	paint();

	setInterval(function() {

		on_new_data_received_ex(`m${clock_tick_count.toFixed(3)};c${Math.floor(Math.random()*1200).toFixed(3)};a${Math.floor(Math.random()*100).toFixed(3)};b${Math.floor(Math.random()*150-50).toFixed(3)};`);

		++clock_tick_count;

	}, 500);

}

function process_databot_data (databot_data) {

	dashboard.chartA.data.setValue(0, 1, databot_data[1]);
	dashboard.chartB.data.setValue(0, 1, databot_data[2]);
	dashboard.chartC.data.setValue(0, 1, databot_data[3]);

	paint();

}

function on_new_data_received_ex (databot_stream) {

	var ax = databot_stream.split(";").slice(0,-1);

	var bx = ax.map(p => parseFloat(p.substring(1)));

	process_databot_data(bx);

}
