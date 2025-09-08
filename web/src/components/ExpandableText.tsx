import React, { useState } from 'react';
import {
  Typography,
  Button,
  Box,
  Collapse,
} from '@mui/material';
import {
  ExpandMore as ExpandMoreIcon,
  ExpandLess as ExpandLessIcon,
} from '@mui/icons-material';

interface ExpandableTextProps {
  text: string;
  maxLength?: number;
  variant?: 'body1' | 'body2' | 'caption' | 'subtitle1' | 'subtitle2';
  color?: 'textPrimary' | 'textSecondary' | 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  expandText?: string;
  collapseText?: string;
  showButton?: boolean;
  buttonVariant?: 'text' | 'outlined' | 'contained';
  buttonSize?: 'small' | 'medium' | 'large';
  buttonColor?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  multiline?: boolean;
  lines?: number;
}

const ExpandableText: React.FC<ExpandableTextProps> = ({
  text,
  maxLength = 100,
  variant = 'body2',
  color = 'textSecondary',
  expandText = 'Show more',
  collapseText = 'Show less',
  showButton = true,
  buttonVariant = 'text',
  buttonSize = 'small',
  buttonColor = 'primary',
  multiline = false,
  lines = 3,
}) => {
  const [expanded, setExpanded] = useState(false);

  const shouldTruncate = text.length > maxLength;
  const displayText = expanded || !shouldTruncate ? text : text.substring(0, maxLength) + '...';

  const handleToggle = () => {
    setExpanded(!expanded);
  };

  if (!shouldTruncate) {
    return (
      <Typography
        variant={variant}
        color={color}
        sx={{
          whiteSpace: multiline ? 'pre-wrap' : 'nowrap',
          overflow: multiline ? 'visible' : 'hidden',
          textOverflow: multiline ? 'unset' : 'ellipsis',
          display: multiline ? 'block' : 'inline',
          ...(multiline && {
            display: '-webkit-box',
            WebkitLineClamp: lines,
            WebkitBoxOrient: 'vertical',
            overflow: 'hidden',
          }),
        }}
      >
        {text}
      </Typography>
    );
  }

  return (
    <Box>
      <Typography
        variant={variant}
        color={color}
        sx={{
          whiteSpace: multiline ? 'pre-wrap' : 'nowrap',
          overflow: multiline ? 'visible' : 'hidden',
          textOverflow: multiline ? 'unset' : 'ellipsis',
          display: multiline ? 'block' : 'inline',
          ...(multiline && {
            display: '-webkit-box',
            WebkitLineClamp: expanded ? 'unset' : lines,
            WebkitBoxOrient: 'vertical',
            overflow: 'hidden',
          }),
        }}
      >
        {displayText}
      </Typography>
      
      {showButton && (
        <Button
          onClick={handleToggle}
          variant={buttonVariant}
          size={buttonSize}
          color={buttonColor}
          startIcon={expanded ? <ExpandLessIcon /> : <ExpandMoreIcon />}
          sx={{
            mt: 1,
            p: 0,
            minWidth: 'auto',
            textTransform: 'none',
          }}
        >
          {expanded ? collapseText : expandText}
        </Button>
      )}
    </Box>
  );
};

export default ExpandableText;
