#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <Arduino.h>
#include <GxEPD2_BW.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/FreeMono9pt7b.h>

class MarkdownParser {
  private:
    GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT>* tft;
    
    // =========================================================================
    // CONFIGURABLE SPACING & STYLING VARIABLES
    // =========================================================================

    // --- Page Margins ---
    int marginX = 2;                // Absolute left margin from the edge of the display (in pixels).
    int marginY = 5;                // Absolute top margin for the very first line of text.
    
    // --- Paragraphs & Text ---
    int x_spacing = 4;              // Fallback space width (now dynamically calculated from font metrics!)
    int y_spacing = 10;             // Vertical gap added after a paragraph, heading, or list completes.
    int y_wrap_spacing = 4;         // Vertical gap between lines of the SAME paragraph when it wraps.
    int standardLineHeight = 12;    // The baseline drop height for standard 9pt text wrapping.
    
    // --- Lists & Blockquotes ---
    int bullet_radius = 2;          // The pixel radius of the filled circle used for bullet points.
    int x_wrap_spacing_bullet = 14; // How far right to indent text so it aligns perfectly after a bullet/number.
    int numbered_list_indent = 24;  // NEW: Minimum text indent for numbered lists to align text vertically.
    int y_wrap_spacing_bullet = 4;  // Vertical wrap gap specifically for list items.
    int blockquote_padding_left = 12; // Indentation for text inside a blockquote.
    int blockquote_line_thickness = 3;// Thickness of the vertical line denoting a blockquote.
    
    // --- Code Blocks ---
    int code_number_width = 16;     // Horizontal space reserved on the left to print line numbers (fits up to 999).
    int code_lang_x_offset = 0;     // Horizontal shift for the language tag (e.g., "cpp") at the start of a block.
    int code_lang_y_gap = 1;        // Vertical gap between the language tag and the first actual line of code.
    int code_padding_left = 6;      // Horizontal space between the vertical margin line and the code text.
    int code_line_thickness = 2;    // The pixel thickness of the vertical line drawn next to the code.
    int code_y_spacing = 1;         // Vertical gap between lines of code (kept tight for readability).
    int code_font_height = 8;       // The physical height of the tiny Org_01 font for calculating drops.
    int code_line_y_offset = -6;    // Y-adjustment to align the vertical margin line with the code's baseline.
    int code_start_y_spacing = -8;  // Pulls the code block up slightly to fix the large visual gap from the previous paragraph.
    int code_end_y_spacing = 16;    // Vertical gap added to the document after a code block completely finishes.
    
    // =========================================================================

    int cursorX;
    int cursorY;
    int currentIndent;   
    int currentWrapLine; 
    int currentLineHeight; 
    int headerLevel;       
    int codeLineCount; 
    int codeBlockIndent; 
    int pendingEmptyLines; 
    int activeListIndent;  
    
    bool inCodeBlock;
    bool isBold;
    bool isItalic;
    bool isInlineCode; 
    bool isStrikethrough;  
    bool isBlockquote;     
    
    int screenWidth;
    int scrollY = 0;     
    int documentHeight = 0; 
    bool moreContent = false;

    // --- NEW: Fix missing typography due to Adafruit's ASCII-only fonts ---
    void cleanTypography(String &str) {
      str.replace("’", "'");
      str.replace("‘", "'");
      str.replace("“", "\"");
      str.replace("”", "\"");
      str.replace("—", "-"); 
      str.replace("–", "-");
      str.replace("…", "...");
    }

    void convertHTMLtoMarkdown(String &str) {
      str.replace("<b>", "**");
      str.replace("</b>", "**");
      str.replace("<i>", "*");
      str.replace("</i>", "*");
    }

    void cleanLinksAndImages(String &str) {
      int imgStart = str.indexOf("![");
      while (imgStart != -1) {
          int linkMid = str.indexOf("](", imgStart);
          int linkEnd = str.indexOf(")", linkMid);
          if (linkMid != -1 && linkEnd != -1) {
              String altText = str.substring(imgStart + 2, linkMid);
              String replacement = "[IMG: " + altText + "]";
              str = str.substring(0, imgStart) + replacement + str.substring(linkEnd + 1);
          } else { break; }
          imgStart = str.indexOf("![");
      }
      
      int linkStart = str.indexOf("[");
      while (linkStart != -1) {
          if (str.substring(linkStart, linkStart + 5) == "[IMG:") {
              linkStart = str.indexOf("[", linkStart + 1);
              continue;
          }
          int linkMid = str.indexOf("](", linkStart);
          int linkEnd = str.indexOf(")", linkMid);
          if (linkMid != -1 && linkEnd != -1 && linkMid > linkStart) {
              String linkText = str.substring(linkStart + 1, linkMid);
              str = str.substring(0, linkStart) + linkText + str.substring(linkEnd + 1);
          } else { break; }
          linkStart = str.indexOf("[", linkStart + 1);
      }
    }

    void setInlineFont() {
      if (headerLevel == 1 || headerLevel == 2) tft->setFont(&FreeSerifBold12pt7b);
      else if (headerLevel == 3) tft->setFont(&FreeSerifBold9pt7b);
      else if (isInlineCode) tft->setFont(&FreeMono9pt7b);
      else if (isBold && isItalic) tft->setFont(&FreeSerifBoldItalic9pt7b);
      else if (isBold) tft->setFont(&FreeSerifBold9pt7b);
      else if (isItalic || isBlockquote) tft->setFont(&FreeSerifItalic9pt7b); 
      else tft->setFont(&FreeSerif9pt7b);
    }

    void drawTextIfVisible(String text, int x, int y, int w, int h) {
      int drawY = y - scrollY;
      if (drawY > -20 && drawY < tft->height() + 20) {
        tft->setCursor(x, drawY);
        tft->print(text);
        
        if (isStrikethrough) {
            int strikeY = drawY - (h / 2) + 2; 
            tft->drawLine(x, strikeY, x + w, strikeY, GxEPD_BLACK);
        }
      }
    }

    void printWord(String word) {
      if (word.length() == 0) return;

      setInlineFont();
      
      // Calculate true font advance using off-screen trick
      tft->setCursor(-1000, -1000);
      tft->print(word);
      int advance = tft->getCursorX() - (-1000);

      // Still need bounding box for vertical alignment & strikethroughs
      int16_t x1, y1; uint16_t w, h;
      tft->getTextBounds(word, 0, 0, &x1, &y1, &w, &h);

      if (cursorX + advance <= screenWidth - marginX) {
        drawTextIfVisible(word, cursorX, cursorY, w, h);
        cursorX += advance;
        return;
      }

      if (cursorX > currentIndent) {
          cursorX = currentIndent;               
          cursorY += currentLineHeight + currentWrapLine;      
      }

      if (cursorX + advance <= screenWidth - marginX) {
        drawTextIfVisible(word, cursorX, cursorY, w, h);
        cursorX += advance;
        return;
      }

      String currentPart = "";
      for (int i = 0; i < word.length(); i++) {
        String nextPart = currentPart + word[i];
        
        tft->setCursor(-1000, -1000);
        tft->print(nextPart);
        int nextAdvance = tft->getCursorX() - (-1000);

        if (cursorX + nextAdvance > screenWidth - marginX) {
           tft->getTextBounds(currentPart, 0, 0, &x1, &y1, &w, &h);
           drawTextIfVisible(currentPart, cursorX, cursorY, w, h);
           
           cursorX = currentIndent;
           cursorY += currentLineHeight + currentWrapLine;
           currentPart = String(word[i]);
        } else {
           currentPart = nextPart;
        }
      }
      
      if (currentPart.length() > 0) {
          tft->setCursor(-1000, -1000);
          tft->print(currentPart);
          int finalAdvance = tft->getCursorX() - (-1000);
          
          tft->getTextBounds(currentPart, 0, 0, &x1, &y1, &w, &h);
          drawTextIfVisible(currentPart, cursorX, cursorY, w, h);
          cursorX += finalAdvance;
      }
    }

    void renderInline(String text) {
      String buffer = "";
      for (int i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == '\\' && i + 1 < text.length()) {
            buffer += text[i+1];
            i++; 
        }
        else if (c == '`') {
          printWord(buffer); buffer = "";
          isInlineCode = !isInlineCode;
        }
        else if (c == '~' && i + 1 < text.length() && text[i+1] == '~') {
          printWord(buffer); buffer = "";
          isStrikethrough = !isStrikethrough;
          i++; 
        }
        // Handle Asterisks OR Underscores for bold/italic formatting
        else if ((c == '*' || c == '_') && i + 2 < text.length() && text[i+1] == c && text[i+2] == c) {
          printWord(buffer); buffer = ""; 
          isBold = !isBold; 
          isItalic = !isItalic;              
          i += 2; 
        }
        else if ((c == '*' || c == '_') && i + 1 < text.length() && text[i+1] == c) {
          printWord(buffer); buffer = ""; 
          isBold = !isBold;               
          i++;                           
        } 
        else if (c == '*' || c == '_') {
          printWord(buffer); buffer = "";
          isItalic = !isItalic;
        } 
        else if (c == ' ') {
          printWord(buffer); buffer = "";
          
          // Calculate true space width for the current font (fixes Mono vs Italic gaps)
          setInlineFont();
          tft->setCursor(-1000, -1000);
          tft->print(" ");
          int spaceWidth = tft->getCursorX() - (-1000);
          if (spaceWidth <= 0) spaceWidth = x_spacing; 
          
          cursorX += spaceWidth; 
          
          if (cursorX > screenWidth - marginX) {
             cursorX = currentIndent;
             cursorY += currentLineHeight + currentWrapLine; 
          }
        } 
        else {
          buffer += c;
        }
      }
      printWord(buffer); 
    }

  public:
    MarkdownParser(GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT>* displayRef) {
      tft = displayRef;
    }

    void begin() { screenWidth = tft->width(); }

    void resetForRender() {
      tft->setTextColor(GxEPD_BLACK);
      tft->setTextWrap(false); 
      
      cursorX = marginX;
      cursorY = marginY + standardLineHeight; 
      currentIndent = marginX;
      currentWrapLine = y_wrap_spacing;
      currentLineHeight = standardLineHeight;
      inCodeBlock = false;
      isInlineCode = false;
      isStrikethrough = false;
      isBlockquote = false;
      headerLevel = 0;
      codeLineCount = 0;
      codeBlockIndent = 0;
      pendingEmptyLines = 0;
      activeListIndent = 0; 
      documentHeight = 0;
    }

    bool scrollDown(int amount) {
      int maximum = max(0, documentHeight - tft->height() + 24);
      if (!moreContent && scrollY >= maximum) return false;
      scrollY = moreContent ? scrollY + amount : min(maximum, scrollY + amount);
      return true;
    }

    // Keeps button-driven upward navigation inside the document instead of
    // rendering negative document coordinates above the e-paper viewport.
    void scrollUp(int amount) {
      scrollY -= amount;
      if (scrollY < 0) { scrollY = 0; }
    }

    void resetScroll() { scrollY = 0; moreContent = true; }

    bool pastViewport(int extraPixels = 48) const {
      return cursorY - scrollY > tft->height() + extraPixels;
    }

    void setMoreContent(bool value) { moreContent = value; }
    
    void setTopOffset(int extraPixels) {
        marginY = 5 + extraPixels;
    }

    void parseBlock(String line) {
      cleanTypography(line); // Ensure smart quotes/dashes are stripped immediately
      
      int leadingSpaces = 0;
      while (leadingSpaces < line.length() && line[leadingSpaces] == ' ') {
          leadingSpaces++;
      }

      String trimmedLine = line;
      trimmedLine.trim();

      if (inCodeBlock && codeBlockIndent > 0 && trimmedLine.length() > 0 && !trimmedLine.startsWith("```")) {
          if (leadingSpaces < codeBlockIndent) {
              inCodeBlock = false;       
              pendingEmptyLines = 0; 
              cursorY += code_end_y_spacing;  
              if (cursorY > documentHeight) documentHeight = cursorY;    
          }
      }

      if (!inCodeBlock && !trimmedLine.startsWith("```")) {
        convertHTMLtoMarkdown(line);
        cleanLinksAndImages(line);
      }

      isBold = false;
      isItalic = false;
      isInlineCode = false;
      isStrikethrough = false;
      isBlockquote = false;
      headerLevel = 0;
      currentIndent = marginX;             
      currentWrapLine = y_wrap_spacing;    
      currentLineHeight = standardLineHeight; 
      
      if (trimmedLine == "---" || trimmedLine == "***" || trimmedLine == "___" || 
          trimmedLine == "- - -" || trimmedLine == "* * *") {
        int drawY = cursorY - scrollY - 6; 
        if (drawY > 0 && drawY < tft->height()) {
          tft->drawLine(marginX, drawY, screenWidth - marginX, drawY, GxEPD_BLACK);
        }
        cursorY += y_spacing + 4;
        activeListIndent = 0; 
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      if (trimmedLine.startsWith("> ")) {
          isBlockquote = true;
          int startY = cursorY - standardLineHeight; 
          
          currentIndent = marginX + blockquote_padding_left + blockquote_line_thickness;
          cursorX = currentIndent;
          
          String quoteText = trimmedLine.substring(2);
          renderInline(quoteText);
          
          int drawStartY = startY - scrollY - standardLineHeight + 4; 
          int drawEndY = cursorY - scrollY + 4;
          
          if (drawEndY > -20 && drawStartY < tft->height() + 20) {
              tft->fillRect(marginX + 2, max(0, drawStartY), blockquote_line_thickness, (drawEndY - drawStartY), GxEPD_BLACK);
          }
          
          cursorY += y_spacing;
          activeListIndent = 0; 
          if (cursorY > documentHeight) documentHeight = cursorY;
          return;
      } else {
          isBlockquote = false;
      }

      if (trimmedLine.startsWith("```")) {
        inCodeBlock = !inCodeBlock;
        if (inCodeBlock) {
          codeLineCount = 0; 
          pendingEmptyLines = 0;
          codeBlockIndent = leadingSpaces; 
          
          cursorY += code_start_y_spacing;
          
          String lang = trimmedLine.substring(3); 
          if (lang.length() > 0) {
            tft->setFont(&Org_01);
            int drawY = cursorY - scrollY;
            if (drawY > 0 && drawY < tft->height()) {
               tft->setCursor(marginX + code_number_width + code_lang_x_offset, drawY);
               tft->print(lang);
            }
            cursorY += code_font_height + code_lang_y_gap; 
          }
        } else {
          pendingEmptyLines = 0; 
          cursorY += code_end_y_spacing;
        }
        activeListIndent = 0; 
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      if (inCodeBlock) {
        int len = line.length();
        while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
            len--;
        }
        line = line.substring(0, len);
        line.replace("\t", "    "); 

        if (line.length() == 0 || trimmedLine.length() == 0) {
            pendingEmptyLines++;
            return; 
        }

        while (pendingEmptyLines > 0) {
            codeLineCount++;
            if (codeLineCount == 1000) {
                tft->setFont(&Org_01);
                int drawY = cursorY - scrollY;
                if (drawY > -20 && drawY < tft->height() + 20) {
                   tft->setCursor(marginX + code_number_width + code_padding_left, drawY);
                   tft->print("[Max 999 lines]");
                }
                cursorY += code_font_height + code_y_spacing;
                if (cursorY > documentHeight) documentHeight = cursorY;
                pendingEmptyLines = 0;
                break;
            } else if (codeLineCount < 1000) {
                tft->setFont(&Org_01);
                int lineX = marginX + code_number_width;
                int drawY = cursorY - scrollY;

                if (drawY > -20 && drawY < tft->height() + 20) {
                   tft->setCursor(marginX, drawY);
                   tft->print(String(codeLineCount));
                   tft->fillRect(lineX, drawY + code_line_y_offset, code_line_thickness, code_font_height + code_y_spacing, GxEPD_BLACK);
                }
                cursorY += code_font_height + code_y_spacing;
                if (cursorY > documentHeight) documentHeight = cursorY;
            }
            pendingEmptyLines--;
        }

        codeLineCount++;
        
        if (codeLineCount > 999) {
          if (codeLineCount == 1000) {
            tft->setFont(&Org_01);
            int drawY = cursorY - scrollY;
            if (drawY > -20 && drawY < tft->height() + 20) {
               tft->setCursor(marginX + code_number_width + code_padding_left, drawY);
               tft->print("[Max 999 lines]");
            }
            cursorY += code_font_height + code_y_spacing;
            if (cursorY > documentHeight) documentHeight = cursorY;
          }
          return; 
        }

        tft->setFont(&Org_01); 
        int lineX = marginX + code_number_width;
        int textX = lineX + code_line_thickness + code_padding_left;
        int currentCodeX = textX;

        int drawY = cursorY - scrollY;

        if (drawY > -20 && drawY < tft->height() + 20) {
           tft->setCursor(marginX, drawY);
           tft->print(String(codeLineCount));
        }

        if (drawY > -20 && drawY < tft->height() + 20) {
           tft->fillRect(lineX, drawY + code_line_y_offset, code_line_thickness, code_font_height + code_y_spacing, GxEPD_BLACK);
        }

        for (int i = 0; i < line.length(); ) {
            String word = "";
            bool isSpace = (line[i] == ' ');

            if (isSpace) {
                word = " ";
                i++;
            } else {
                while (i < line.length() && line[i] != ' ') {
                    word += line[i];
                    i++;
                }
            }

            if (isSpace) {
                tft->setCursor(-1000, -1000);
                tft->print(" ");
                int adv = tft->getCursorX() - (-1000);
                if (adv <= 0) adv = 4;
                
                if (currentCodeX + adv > screenWidth - marginX) {
                    if (currentCodeX > textX) {
                        currentCodeX = textX; 
                        cursorY += code_font_height + code_y_spacing;
                        drawY = cursorY - scrollY;
                        if (drawY > -20 && drawY < tft->height() + 20) {
                            tft->fillRect(lineX, drawY + code_line_y_offset, code_line_thickness, code_font_height + code_y_spacing, GxEPD_BLACK);
                        }
                    }
                } else {
                    currentCodeX += adv;
                }
            } 
            else {
                tft->setCursor(-1000, -1000);
                tft->print(word);
                int adv = tft->getCursorX() - (-1000);
                
                if (currentCodeX + adv > screenWidth - marginX) {
                    if (currentCodeX > textX) {
                        currentCodeX = textX; 
                        cursorY += code_font_height + code_y_spacing;
                        drawY = cursorY - scrollY;
                        
                        if (drawY > -20 && drawY < tft->height() + 20) {
                            tft->fillRect(lineX, drawY + code_line_y_offset, code_line_thickness, code_font_height + code_y_spacing, GxEPD_BLACK);
                        }
                    }
                    
                    if (currentCodeX + adv > screenWidth - marginX) {
                        String currentPart = "";
                        for (int j = 0; j < word.length(); j++) {
                            String nextPart = currentPart + word[j];
                            tft->setCursor(-1000, -1000);
                            tft->print(nextPart);
                            int nAdv = tft->getCursorX() - (-1000);

                            if (currentCodeX + nAdv > screenWidth - marginX) {
                                drawY = cursorY - scrollY;
                                if (drawY > -20 && drawY < tft->height() + 20) {
                                    tft->setCursor(currentCodeX, drawY);
                                    tft->print(currentPart);
                                }

                                currentCodeX = textX;
                                cursorY += code_font_height + code_y_spacing;
                                drawY = cursorY - scrollY;
                                if (drawY > -20 && drawY < tft->height() + 20) {
                                    tft->fillRect(lineX, drawY + code_line_y_offset, code_line_thickness, code_font_height + code_y_spacing, GxEPD_BLACK);
                                }
                                currentPart = String(word[j]);
                            } else {
                                currentPart = nextPart;
                            }
                        }
                        if (currentPart.length() > 0) {
                            drawY = cursorY - scrollY;
                            if (drawY > -20 && drawY < tft->height() + 20) {
                                tft->setCursor(currentCodeX, drawY);
                                tft->print(currentPart);
                            }
                            tft->setCursor(-1000, -1000);
                            tft->print(currentPart);
                            currentCodeX += tft->getCursorX() - (-1000);
                        }
                        word = ""; 
                    }
                }
                
                if (word.length() > 0) {
                    if (drawY > -20 && drawY < tft->height() + 20) {
                        tft->setCursor(currentCodeX, drawY);
                        tft->print(word);
                    }
                    currentCodeX += adv;
                }
            }
        }
        cursorY += code_font_height + code_y_spacing; 
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      if (trimmedLine.startsWith("# ") || trimmedLine.startsWith("## ") || trimmedLine.startsWith("### ")) {
        int textStart = 0;
        
        if (trimmedLine.startsWith("### ")) { headerLevel = 3; textStart = line.indexOf("### ") + 4; }
        else if (trimmedLine.startsWith("## ")) { headerLevel = 2; textStart = line.indexOf("## ") + 3; }
        else { headerLevel = 1; textStart = line.indexOf("# ") + 2; }
        
        currentLineHeight = (headerLevel == 3) ? 14 : 18;
        
        if (cursorY > marginY + standardLineHeight) {
            cursorY += y_spacing;
        }

        cursorX = marginX;
        renderInline(line.substring(textStart));
        cursorY += currentLineHeight + y_spacing; 
        
        activeListIndent = 0; 
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      if (trimmedLine.startsWith("- ") || trimmedLine.startsWith("* ")) {
        int listIndent = marginX + (leadingSpaces * 4); 
        currentIndent = listIndent + x_wrap_spacing_bullet; 
        
        activeListIndent = currentIndent;
        
        bool isTask = false;
        bool isTaskDone = false;
        int textStart = line.indexOf(trimmedLine.charAt(0)) + 2; 
        
        if (trimmedLine.substring(2).startsWith("[ ] ")) {
            isTask = true; 
            textStart += 4;
        } else if (trimmedLine.substring(2).startsWith("[x] ") || trimmedLine.substring(2).startsWith("[X] ")) {
            isTask = true; 
            isTaskDone = true; 
            textStart += 4;
        }
        
        int drawY = cursorY - scrollY;
        if (drawY > 0 && drawY < tft->height()) {
           if (isTask) {
               int boxSize = 8;
               int boxY = drawY - boxSize - 1;
               tft->drawRect(listIndent + 2, boxY, boxSize, boxSize, GxEPD_BLACK);
               if (isTaskDone) {
                   tft->drawLine(listIndent + 2, boxY, listIndent + 2 + boxSize, boxY + boxSize, GxEPD_BLACK);
                   tft->drawLine(listIndent + 2 + boxSize, boxY, listIndent + 2, boxY + boxSize, GxEPD_BLACK);
               }
           } else {
               tft->fillCircle(listIndent + 4, drawY - 4, bullet_radius, GxEPD_BLACK); 
           }
        }
        
        cursorX = currentIndent; 
        renderInline(line.substring(textStart));
        cursorY += standardLineHeight + y_spacing; 
        
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      // --- DYNAMIC NUMBERED LIST PARSING ---
      int dotSpaceIdx = trimmedLine.indexOf(". ");
      bool isNumberedList = false;
      if (dotSpaceIdx > 0) {
          isNumberedList = true;
          for (int i = 0; i < dotSpaceIdx; i++) {
              if (!isDigit(trimmedLine[i])) {
                  isNumberedList = false;
                  break;
              }
          }
      }

      if (isNumberedList) {
        int listIndent = marginX + (leadingSpaces * 4);
        
        setInlineFont(); 
        String numStr = trimmedLine.substring(0, dotSpaceIdx + 2);
        
        // Dynamically measure the exact width of the printed number (e.g. "10. ")
        tft->setCursor(-1000, -1000);
        tft->print(numStr);
        int numAdvance = tft->getCursorX() - (-1000);
        if (numAdvance <= 0) numAdvance = x_wrap_spacing_bullet; // Safety fallback
        
        // --- NEW: RIGHT-ALIGN NUMBERS FOR PERFECT TEXT COLUMN ---
        int textOffset = (numAdvance > numbered_list_indent) ? numAdvance : numbered_list_indent;
        currentIndent = listIndent + textOffset; 
        activeListIndent = currentIndent;
        
        int drawY = cursorY - scrollY;
        if (drawY > -20 && drawY < tft->height() + 20) {
           tft->setCursor(currentIndent - numAdvance, drawY); // Right-aligned against the column!
           tft->print(numStr); 
        }
        
        cursorX = currentIndent; 
        int textStart = line.indexOf(numStr);
        renderInline(line.substring(textStart + numStr.length())); 
        cursorY += standardLineHeight + y_spacing; 
        
        if (cursorY > documentHeight) documentHeight = cursorY;
        return;
      }

      if (trimmedLine.length() > 0) {
        if (leadingSpaces == 0 && activeListIndent > 0) {
            currentIndent = activeListIndent;
        } else {
            currentIndent = marginX + (leadingSpaces * 4);
            activeListIndent = currentIndent; 
        }
        
        cursorX = currentIndent;
        renderInline(line);
        cursorY += standardLineHeight + y_spacing; 
        
        if (cursorY > documentHeight) documentHeight = cursorY;
      } else {
        activeListIndent = 0; 
        
        if (cursorY > marginY + standardLineHeight) {
            cursorX = marginX;
            cursorY += standardLineHeight; 
            if (cursorY > documentHeight) documentHeight = cursorY;
        }
      }
    }
};

#endif
