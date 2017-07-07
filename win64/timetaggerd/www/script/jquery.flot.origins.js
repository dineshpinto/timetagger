(function() {

    function dashedLine(ctx, x, y, x2, y2, da) {
        if (!da) da = [10,5];
        ctx.save();
        var dx = (x2-x), dy = (y2-y);
        var len = Math.sqrt(dx*dx + dy*dy);
        var rot = Math.atan2(dy, dx);
        ctx.translate(x, y);
        ctx.moveTo(0, 0);
        ctx.rotate(rot);       
        var dc = da.length;
        var di = 0, draw = true;
        x = 0;
        while (len > x) {
            x += da[di++ % dc];
            if (x > len) x = len;
            draw ? ctx.lineTo(x, 0): ctx.moveTo(x, 0);
            draw = !draw;
        }       
        ctx.restore();
    }
    	
	function draw_origins(plot, ctx, series) {
		var plotOffset = plot.getPlotOffset();
        var axes = plot.getAxes();

        ctx.save();
		
		var plotOffset = plot.getPlotOffset();
		var axes = plot.getAxes();

		if (axes.xaxis.options.drawOrigin) {
			var x=axes.xaxis.p2c(0);
			if (x>1) {
				var yf = axes.yaxis.p2c(axes.yaxis.min);
				var yt = axes.yaxis.p2c(axes.yaxis.max);
		
				ctx.translate(plotOffset.left, plotOffset.top);
	
				ctx.beginPath();
				ctx.strokeStyle = "#ddd";
				dashedLine(ctx, x, yf, x, yt, [4,4]);	
				ctx.stroke();
				ctx.restore();        
			}
		}	
		if (axes.yaxis.options.drawOrigin) {
			var y=axes.yaxis.p2c(0);
			if (y>1) {
				var xf = axes.xaxis.p2c(axes.xaxis.min);
				var xt = axes.xaxis.p2c(axes.xaxis.max);
		
				ctx.translate(plotOffset.left, plotOffset.top);
	
				ctx.beginPath();
				ctx.strokeStyle = "#ddd";
				dashedLine(ctx, xf, y, xt, y, [4,4]);	
				ctx.stroke();
				ctx.restore();        
			}
		}	
	}
	
    function init(plot) {
        plot.hooks.drawSeries.push(draw_origins);
    };

    var options = {
            series: {
                origins: true
    		}
    	};
	
    	
    $.plot.plugins.push({
        init: init,
        name: "origins",
        options: options,
        version: "0.1"
    });	
})();
