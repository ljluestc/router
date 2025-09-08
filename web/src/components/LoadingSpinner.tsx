import React from 'react';
import {
  Box,
  CircularProgress,
  Typography,
  Paper,
  Fade,
} from '@mui/material';

interface LoadingSpinnerProps {
  size?: number;
  message?: string;
  color?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  thickness?: number;
  variant?: 'determinate' | 'indeterminate';
  value?: number;
  showMessage?: boolean;
  fullScreen?: boolean;
  overlay?: boolean;
  fadeIn?: boolean;
  fadeInDelay?: number;
}

const LoadingSpinner: React.FC<LoadingSpinnerProps> = ({
  size = 40,
  message = 'Loading...',
  color = 'primary',
  thickness = 4,
  variant = 'indeterminate',
  value = 0,
  showMessage = true,
  fullScreen = false,
  overlay = false,
  fadeIn = true,
  fadeInDelay = 0,
}) => {
  const [visible, setVisible] = React.useState(!fadeIn);

  React.useEffect(() => {
    if (fadeIn) {
      const timer = setTimeout(() => {
        setVisible(true);
      }, fadeInDelay);

      return () => clearTimeout(timer);
    }
  }, [fadeIn, fadeInDelay]);

  const content = (
    <Box
      display="flex"
      flexDirection="column"
      alignItems="center"
      justifyContent="center"
      gap={2}
      p={3}
    >
      <CircularProgress
        size={size}
        color={color}
        thickness={thickness}
        variant={variant}
        value={value}
      />
      {showMessage && (
        <Typography
          variant="body2"
          color="text.secondary"
          textAlign="center"
        >
          {message}
        </Typography>
      )}
    </Box>
  );

  if (fullScreen) {
    return (
      <Box
        position="fixed"
        top={0}
        left={0}
        right={0}
        bottom={0}
        display="flex"
        alignItems="center"
        justifyContent="center"
        bgcolor={overlay ? 'rgba(0, 0, 0, 0.5)' : 'background.paper'}
        zIndex={9999}
      >
        {fadeIn ? (
          <Fade in={visible} timeout={300}>
            <Paper elevation={3} sx={{ p: 4, borderRadius: 2 }}>
              {content}
            </Paper>
          </Fade>
        ) : (
          <Paper elevation={3} sx={{ p: 4, borderRadius: 2 }}>
            {content}
          </Paper>
        )}
      </Box>
    );
  }

  if (overlay) {
    return (
      <Box
        position="absolute"
        top={0}
        left={0}
        right={0}
        bottom={0}
        display="flex"
        alignItems="center"
        justifyContent="center"
        bgcolor="rgba(0, 0, 0, 0.5)"
        zIndex={1000}
      >
        {fadeIn ? (
          <Fade in={visible} timeout={300}>
            <Paper elevation={3} sx={{ p: 4, borderRadius: 2 }}>
              {content}
            </Paper>
          </Fade>
        ) : (
          <Paper elevation={3} sx={{ p: 4, borderRadius: 2 }}>
            {content}
          </Paper>
        )}
      </Box>
    );
  }

  if (fadeIn) {
    return (
      <Fade in={visible} timeout={300}>
        <Box>
          {content}
        </Box>
      </Fade>
    );
  }

  return content;
};

export default LoadingSpinner;
