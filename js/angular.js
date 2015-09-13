var app = angular.module("name_module_angular", ['ui.bootstrap', 'angularUtils.directives.dirPagination'])
app.controller("name_controller", function($scope, $http) {
	var ns = this;
	ns.cache = {};
	ns.debug = "";
	
	ns.onUpdateRest = function() {
		ns.webservice = "https://restcountries.eu/rest/v1/" + ns.region;
		$http.get(ns.webservice).then(
			function(results) {
				ns.cache = results.data;
			},
			function(data) {
				console.log("Error restcountries.eu")
			}
		);
	}

	ns.onSearch = function() {
		ns.filter = ns.filter_intend;
	}
	
	ns.region = "all";

	$scope.$watch('ns.region', function(oldVal, newVal) {
		ns.onUpdateRest();
		ns.debug = "changed from " + newVal + " to " + oldVal;
	});

	  ns.totalItems = 64;
	  ns.currentPage = 4;

	  ns.setPage = function (pageNo) {
		ns.currentPage = pageNo;
	  };

	  ns.pageChanged = function() {
		console.log('Page changed to: ' + ns.currentPage);
	  };

	
	  ns.numPages = 10;
	  ns.maxSize = 5;
	  ns.bigTotalItems = 175;
	  ns.bigCurrentPage = 1;
});
