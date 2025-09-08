import React from 'react';
import {
  Box,
  Typography,
  LinearProgress,
  CircularProgress,
  Paper,
} from '@mui/material';

export type ProgressVariant = 'linear' | 'circular';

interface ProgressBarProps {
  value: number;
  max?: number;
  variant?: ProgressVariant;
  size?: 'small' | 'medium' | 'large';
  color?: 'primary' | 'secondary' | 'success' | 'warning' | 'error' | 'info';
  showLabel?: boolean;
  label?: string;
  showPercentage?: boolean;
  showValue?: boolean;
  thickness?: number;
  width?: number | string;
  height?: number | string;
  animated?: boolean;
  striped?: boolean;
  rounded?: boolean;
}

const ProgressBar: React.FC<ProgressBarProps> = ({
  value,
  max = 100,
  variant = 'linear',
  size = 'medium',
  color = 'primary',
  showLabel = false,
  label,
  showPercentage = true,
  showValue = false,
  thickness,
  width,
  height,
  animated = false,
  striped = false,
  rounded = false,
}) => {
  const percentage = Math.min(Math.max((value / max) * 100, 0), 100);

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          linearHeight: 4,
          circularSize: 24,
          fontSize: '0.75rem',
        };
      case 'large':
        return {
          linearHeight: 8,
          circularSize: 48,
          fontSize: '1.1rem',
        };
      default: // medium
        return {
          linearHeight: 6,
          circularSize: 32,
          fontSize: '0.875rem',
        };
    }
  };

  const sizeProps = getSizeProps();

  const getLabel = () => {
    if (label) return label;
    if (showValue) return `${value}${max ? ` / ${max}` : ''}`;
    if (showPercentage) return `${Math.round(percentage)}%`;
    return '';
  };

  const renderLinearProgress = () => (
    <Box
      sx={{
        width: width || '100%',
        height: height || sizeProps.linearHeight,
        position: 'relative',
        ...(rounded && {
          borderRadius: sizeProps.linearHeight / 2,
          overflow: 'hidden',
        }),
      }}
    >
      <LinearProgress
        variant="determinate"
        value={percentage}
        color={color}
        thickness={thickness || sizeProps.linearHeight}
        sx={{
          height: '100%',
          ...(striped && {
            '& .MuiLinearProgress-bar': {
              backgroundImage: `linear-gradient(45deg, transparent 25%, rgba(255,255,255,0.2) 25%, rgba(255,255,255,0.2) 50%, transparent 50%, transparent 75%, rgba(255,255,255,0.2) 75%)`,
              backgroundSize: '20px 20px',
              animation: animated ? 'stripes 1s linear infinite' : 'none',
            },
          }),
          ...(animated && {
            '& .MuiLinearProgress-bar': {
              transition: 'transform 0.3s ease-in-out',
            },
          }),
          ...(rounded && {
            borderRadius: sizeProps.linearHeight / 2,
          }),
        }}
      />
      {showLabel && (
        <Box
          sx={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)',
            color: 'white',
            fontSize: sizeProps.fontSize,
            fontWeight: 'bold',
            textShadow: '1px 1px 2px rgba(0,0,0,0.5)',
          }}
        >
          {getLabel()}
        </Box>
      )}
    </Box>
  );

  const renderCircularProgress = () => (
    <Box
      sx={{
        position: 'relative',
        display: 'inline-flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: width || sizeProps.circularSize,
        height: height || sizeProps.circularSize,
      }}
    >
      <CircularProgress
        variant="determinate"
        value={percentage}
        color={color}
        thickness={thickness || 4}
        size={sizeProps.circularSize}
        sx={{
          ...(animated && {
            '& .MuiCircularProgress-circle': {
              transition: 'stroke-dashoffset 0.3s ease-in-out',
            },
          }),
        }}
      />
      {showLabel && (
        <Box
          sx={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)',
            color: 'text.primary',
            fontSize: sizeProps.fontSize,
            fontWeight: 'bold',
          }}
        >
          {getLabel()}
        </Box>
      )}
    </Box>
  );

  const content = variant === 'circular' ? renderCircularProgress() : renderLinearProgress();

  if (showLabel && !label) {
    return (
      <Box>
        {content}
        <Typography
          variant="body2"
          color="text.secondary"
          sx={{
            mt: 1,
            textAlign: 'center',
            fontSize: sizeProps.fontSize,
          }}
        >
          {getLabel()}
        </Typography>
      </Box>
    );
  }

  return content;
};

export default ProgressBar;
