<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output encoding="UTF-8" method="html" omit-xml-declaration="yes" indent="yes"/>

    <xsl:template match="tree">
        <html>
            <title>MGRA tree</title>
            <style type="text/css">
                .end0{color:red}
                .end1{color:green}
                .end2{color:magenta}
                .end3{color:orange}
            </style>

            <script>
                var values =  [
                    <xsl:apply-templates select="row/cell/text"/>
                ]

                function showData(show) {
                    for (var i = 0; values.length > i; i++) {
                        var cur = values[i];
                        changeStyle('trs'+cur, show);
                        changeStyle('gen'+cur, show);
                    }
                }

                function changeStyle(id, show){
                    var element = document.getElementById(id);
                    if (element != null) {
                        element.style.display= id == show ? "" : "none";;
                    }
                }
            </script>
            <body>
                <h2>MGRA tree</h2>
                <table border="1" cellpadding="10">
                    <xsl:apply-templates select="row"/>
                </table>
                <xsl:apply-templates select="row/cell/transformations"/>
                <xsl:apply-templates select="row/cell/genome"/>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="row">
        <tr>
             <xsl:apply-templates select="cell"/>
        </tr>
    </xsl:template>

    <xsl:template match="cell">
        <td rowspan="{height}" colspan="{width}" align="center">
            <xsl:apply-templates select="length"/>
            <strong>
                <xsl:choose>
                    <xsl:when test="genome"><a href="#" onclick="showData('gen{text}')"><xsl:value-of select="text"/></a></xsl:when>
                    <xsl:otherwise><xsl:value-of select="text"/></xsl:otherwise>
                </xsl:choose>
                </strong>
        </td>
    </xsl:template>

    <xsl:template match="length">
        <a href="#" onclick="showData('trs{../text}')"><xsl:value-of select="."/></a>
        <br/>
    </xsl:template>

    <xsl:template match="text">
        '<xsl:value-of select="."/>',
    </xsl:template>

    <xsl:template match="transformations">
        <div id="trs{../text}" style="display:none;">
            <h3>Transformations for <xsl:value-of select="../text"/></h3>
            <xsl:apply-templates select="transformation"/>
        </div>
    </xsl:template>

    <xsl:template match="transformation">
        <xsl:apply-templates select="before/chromosome"/>
        <xsl:apply-templates select="end"/>
        <br/>
        <xsl:apply-templates select="after/chromosome"/>
        <br/>
    </xsl:template>

    <xsl:template match="end">
        <span class="end{color}">
            <xsl:value-of select="id"/><xsl:value-of select="type"/>&#160;
        </span>
    </xsl:template>


    <xsl:template match="genome">
        <div id="gen{../text}" style="display:none;">
            <h3>Chromosomes for <xsl:value-of select="../text"/></h3>
            <xsl:apply-templates select="chromosome"/>
        </div>
    </xsl:template>

    <xsl:template match="chromosome">
        <xsl:if test="10>id">&#160;</xsl:if>
        <xsl:value-of select="id"/>.<xsl:apply-templates select="gene"/><br/>
    </xsl:template>

    <xsl:template match="gene">
        <a href="#{id}" title="{id}">
            <xsl:if test="end">
                <xsl:attribute name="class">end<xsl:value-of select="end/color"/></xsl:attribute>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="direction='minus'">&lt;</xsl:when>
                <xsl:otherwise>&gt;</xsl:otherwise>
            </xsl:choose>
        </a>
    </xsl:template>
</xsl:stylesheet>
