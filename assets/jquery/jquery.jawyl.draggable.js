(function($){
	$.extend({
		dragprompt:false,
		dragitem:null,
		dragdata:null,
		droptarget:null,
		draghelper:null,
		startdrag:function(e,item){
			$.dragprompt = false;
			$.dragitem = item;
						
			$.draghelper = $('<div class="draghelper" style="position:absolute;z-index:100;"></div>');
			$.draghelper.appendTo(document.body);
			var cl = $($.dragitem).clone();
			cl.get(0).id = "dh_"+cl.get(0).id;
			var cont = cl.contents();
			for (var i = 0, max = cont.length; i < max; i++)
				cont.get(i).id = "dh_"+cont.get(i).id;
			cl.appendTo($.draghelper);
			$(document.body).addClass("drag-background");
		},	
		stopdrag:function(){
			if ($.draghelper)
				$.draghelper.remove();
			if ($.droptarget)
				$($.droptarget).removeClass("drop-target");	
			$.draghelper = null;
			$.droptarget = null;	
			$.dragprompt = false;
			$.dragitem = null;
			$(document.body).removeClass("drag-background");			
		}
	});
	$.fn.extend({
		draggable:function(){
			this.extend(this.__draggable__);
			return this;
		},
		droppable:function(){
			this.extend(this.__droppable__);
			return this;
		},
		__draggable__:{
			construct:function(data){
				if (data)
					this.data("dragdata",data)
				else
					this.data("dragdata",{});
					
				this.mousedown(function(e){
					if (e.which == 1){ 
						$.dragprompt = true;
						e.preventDefault();
					}
					
				});
				this.mousemove(function(e){
					if (($.dragprompt) && (!$.dragitem))
						$.startdrag(e,$(this));
					if ($.dragitem)
						e.preventDefault();
				});
				this.mouseup(function(e){
					if ($.dragprompt)
						$.dragprompt = false;
					if ($.dragitem){
						$.stopdrag();
						e.preventDefault();;
					}
				});
				return this;
			}
		},
		__droppable__:{
			construct:function(options){
				if (options){
					if (options.ondrop)
						this.ondrop = options.ondrop;
					if (options.oncheckdrop)
						this.oncheckdrop = options.oncheckdrop;
				}
				this.mouseover(function(e){
					if ($.dragitem){
						if ($.dragitem != this)
							$(this).addClass("drop-target");	
						$.droptarget = this;
						e.preventDefault();
					}
				});
				this.mouseout(function(e){
					if ($.dragitem){
						if ($.dragitem != this)
							$(this).removeClass("drop-target");
						$.droptarget = null;
						e.preventDefault();
					}
				});
				this.mouseup(function(e){
					if ($.dragitem){
						if ($.dragitem != this){
							if (this.ondrop){
								if (this.oncheckdrop){
									if (this.oncheckdrop(this,$.dragitem.data("dragdata")))
										this.ondrop($.dragitem.data("dragdata"));
								}
								else
								this.ondrop($.dragitem.data("dragdata"));
							}	
						}
						$.stopdrag();
						e.preventDefault();
					}
				});
			}
		}
	});
	$(document).mousemove(function(e){
		if ($.draghelper){
			$.draghelper.css("left",e.pageX+5);
			$.draghelper.css("top",e.pageY+5);
			e.preventDefault();
		}
	});
	$(document).mouseup(function(e){
		if ($.dragitem){
			$.stopdrag();
			e.preventDefault();
		}
	});
})(jQuery); 