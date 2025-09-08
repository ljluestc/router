import React from 'react';
import {
  Box,
  CircularProgress,
  Typography,
  Paper,
  LinearProgress,
} from '@mui/material';

interface LoadingProps {
  message?: string;
  size?: number;
  variant?: 'circular' | 'linear';
  fullScreen?: boolean;
}

const Loading: React.FC<LoadingProps> = ({
  message = 'Loading...',
  size = 40,
  variant = 'circular',
  fullScreen = false,
}) => {
  const content = (
    <Box
      display="flex"
      flexDirection="column"
      alignItems="center"
      justifyContent="center"
      p={3}
    >
      {variant === 'circular' ? (
        <CircularProgress size={size} sx={{ mb: 2 }} />
      ) : (
        <Box sx={{ width: '100%', maxWidth: 300, mb: 2 }}>
          <LinearProgress />
        </Box>
      )}
      <Typography variant="body1" color="text.secondary">
        {message}
      </Typography>
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
        bgcolor="background.paper"
        zIndex={9999}
      >
        <Paper elevation={3} sx={{ p: 4, borderRadius: 2 }}>
          {content}
        </Paper>
      </Box>
    );
  }

  return content;
};

export default Loading;
