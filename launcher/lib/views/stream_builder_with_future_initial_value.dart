import 'package:flutter/material.dart';

typedef Builder<T> = Widget Function(BuildContext context, T futureValue);

/// This widget loads a future as an initial value and then streams
/// subsequent updates to the data from the StreamBuilder. This is useful
/// if you want to load some initial value asynchronously and update the
/// value from a stream afterwards.
class StreamBuilderWithFutureInitialValue<T> extends StatelessWidget {
  final Future<T>? future;
  final Stream<T>? stream;
  final Widget loader;
  final Builder<T> builder;

  const StreamBuilderWithFutureInitialValue(
      {super.key,
      required this.future,
      required this.stream,
      required this.loader,
      required this.builder});

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
        future: future,
        builder: (BuildContext context, AsyncSnapshot<T> futureSnapshot) {
          if (futureSnapshot.data == null) {
            return loader;
          }

          return StreamBuilder(
              stream: stream,
              builder: (BuildContext context, AsyncSnapshot<T> streamSnapshot) {
                if (streamSnapshot.hasData) {
                  return builder(context, streamSnapshot.data!);
                } else {
                  return builder(context, futureSnapshot.data!);
                }
              });
        });
  }
}
