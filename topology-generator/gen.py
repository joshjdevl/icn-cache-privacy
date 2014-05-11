import jinja2


templateLoader = jinja2.FileSystemLoader( searchpath="." )
templateEnv = jinja2.Environment( loader=templateLoader )

TEMPLATE_FILE = "template.txt"
template = templateEnv.get_template( TEMPLATE_FILE )

line = {'number_consumers':5,'number_producers':10}

outputText = template.render( line )
print outputText
