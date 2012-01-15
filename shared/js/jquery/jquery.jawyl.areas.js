(function($){
	$.extend({
			areas_response_type:"xml",
			areas_date_format:"d/m/y",
			areas_add_tag:"add",
			areas_delete_tag:"delete",
			areas_move_tag:"move",
			areas_refresh_tag:"refresh",
			areas_OnDatePicker:false,
			areas_AjaxCall:function(url,data,sync){
				if (url)
					$.ajax({async:!sync, data: data,type: "POST", dataType: $.areas_response_type, url: url,success:$.areas_StdCallBack});
				else
					$.ajax({async:!sync, data: data,type: "POST", dataType: $.areas_response_type, success:$.areas_StdCallBack});	
			},
			areas_parse_scripts:function(txt){
				var jss = txt.match(/<script[^>]*>[\s\S]*<\/script>/igm);
				if (jss)
					for (var i = 0; i < jss.length; i++)
						eval(jss[i].replace(/<script[^>]*>|<\/script>/igm,""));
			},
			areas_StdCallBack:function(data){
				switch ($.areas_response_type){
					case "xml":{
						error = data.getElementsByTagName("error");
						if (error.length > 0){
							alert(error.item(0).firstChild.nodeValue);
							return;
						}
						areas = data.getElementsByTagName("response").item(0).childNodes;
						for (i = 0; i < areas.length; i++){
							switch (areas.item(i).tagName){
								case $.areas_add_tag:$("#"+areas.item(i).getAttribute("parent")).append(unescape(areas.item(i).firstChild.nodeValue));break;
								case $.areas_refresh_tag:{$("#"+areas.item(i).getAttribute("id")).replaceWith(unescape(areas.item(i).firstChild.nodeValue));}break;
								case $.areas_delete_tag:$("#"+areas.item(i).getAttribute("id")).remove();break;
								case $.areas_move_tag:{
										$("#"+areas.item(i).getAttribute("id")).detach();
										$("#"+areas.item(i).getAttribute("id")).appendTo("#"+areas.item(i).getAttribute("parent"));									
								}break;
							}	
						}
					}break;
					case "html":{
						error = data.match(/\[--error--\](.*?)\[--error--\]/);
						if (error) alert(error[1]);
						else {
							areas = data.split(/(<--area--\{(\w*)\}--\{(\w*)\}--\{(\w*)\}-->)/i);
							for (i = 1; i < areas.length; i = i + 4){
								id = areas[i];
								parentid = areas[i+1];
								action = areas[i+2];
								html = areas[i+3];
								switch (action){
									case $.areas_add_tag:$("#"+parentid).append(html);break;
									case $.areas_refresh_tag:$("#"+id).replaceWith(html);break;
									case $.areas_delete_tag:$("#"+id).remove();break;
									case $.areas_move_tag:{
										$("#"+id).detach();
										$("#"+id).appendTo("#"+parentid);
									}break;
								}
							}	
						}
					}break;
					case "json":{
		    			if (data.error){
		    				alert(data.error);
		    				return;
		    			}
						for (ind in data){
							switch (data[ind][1]){
								case $.areas_add_tag:$("#"+data[ind][3]).append(data[ind][2]);break;
								case $.areas_refresh_tag:$("#"+data[ind][0]).replaceWith(data[ind][2]);break;
								case $.areas_delete_tag:$("#"+data[ind][0]).remove();break;
								case $.areas_move_tag:{
										$("#"+data[ind][0]).detach();
										$("#"+data[ind][0]).appendTo("#"+data[ind][3]);
								}break;
							}
						}
					}break;
				}
			}
	});
	$.fn.extend({
		areas_ajax_forms:function(){
			$("form",this).ajaxForm({target:null,dataType:$.areas_response_type,success:$.areas_StdCallBack});
			$("form",this).submit(function(e){$(this).ajaxSubmit({dataType:$.areas_response_type, success:$.areas_StdCallBack});return false;});
		},
		areas_editor:function(options){
			var el;
			if (options.inputs){
				if (options.inputs.length > 0)
					el = $(options.inputs[0],this);
				for (var i = 1; i < options.inputs.length; i++)
					el.add(options.inputs[i],this);
			}
			else
			 el = $("select",this).add("input:checkbox",this).add("input:text",this).add("input:password",this).add("textarea");
			 
			var datepickers = $("input.date",this);
			if (datepickers.length > 0){
				if (options.ondatepicker)
					options.ondatepicker(datepickers);
				else
					if ($.areas_OnDatePicker)
						$.areas_OnDatePicker(datepickers);
			}
			
			$("div.file",this).fileupload().construct({
					ResponseDataType:$.areas_response_type,
					OnUpload:function(e){eval("var data="+e.response);$.areas_StdCallBack(data);},
					OnDeleted:$.areas_StdCallBack
			});
			
			$("div.image",this).imageupload().construct({
					ResponseDataType:$.areas_response_type,
					OnUpload:function(e){eval("var data="+e.response);$.areas_StdCallBack(data);},
					OnDeleted:$.areas_StdCallBack
			});
			
			var wo = options.wyswyg;
			if (!wo) wo = {};
			
			var htmleditors = $("textarea.html",this);
			if (htmleditors.length > 0)
				if (options.onhtmleditor)
					options.onhtmleditor(htmleditors);
			
			var po = options.poston;			 
			if (!po)
				po = [];
			for (var i = 0; i < po.length; i++)
				el.bind(po[i],function(){$(this.form).submit();return false;});
		}
	});	
})(jQuery);